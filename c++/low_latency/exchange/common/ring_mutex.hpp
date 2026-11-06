#pragma once
// Baseline SPSC ring buffer: one POSIX mutex protects head, tail and count.
// This is the straightforward "first version"; faster versions replace it later (task 27).
//
// Layout is one contiguous block with no pointers inside, so the same block can later
// be placed in shared memory and used by two processes:
//
//   [ MutexRing header: mutex, capacity, head, tail, count ][ slot 0 ][ slot 1 ] ... [ slot N-1 ]
//
// The mutex is created PTHREAD_PROCESS_SHARED: std::mutex has no way to say that,
// so it would only work between threads, not between processes.
//
//   void* mem = ...;                                  // at least bytesFor(capacity) bytes
//   auto* ring = MutexRing<Msg>::create(mem, 1024, &err);
//   ring->tryPush(msg);  ring->tryPop(msg);            // false = full / empty, caller decides how to wait
//   auto* same = MutexRing<Msg>::attach(mem, bytes, &err);  // another process: use it, don't re-initialise
//
// A process-shared mutex has one danger: if a process dies while holding it, the other side
// waits for ever. Linux "robust" mutexes (EOWNERDEAD) could recover; we don't, because a crashed
// stage means restarting the pipeline anyway, and the lock-free rings (task 27) have no lock.

#include <pthread.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <string>
#include <type_traits>

namespace lab {

template <typename T>
class MutexRing {
    static_assert(std::is_trivially_copyable_v<T>, "ring slots are copied with memcpy");

public:
    // Bytes needed for the header plus `capacity` slots
    static size_t bytesFor(uint32_t capacity) { return sizeof(MutexRing) + size_t(capacity) * sizeof(T); }

    // Builds a ring inside `mem` (at least bytesFor(capacity) bytes). nullptr + *err on failure.
    static MutexRing* create(void* mem, uint32_t capacity, std::string* err) {
        static_assert(sizeof(MutexRing) % alignof(T) == 0, "slots must start correctly aligned after the header");
        if (capacity == 0) return fail(err, "capacity must be > 0");
        if (reinterpret_cast<uintptr_t>(mem) % alignof(MutexRing) != 0) return fail(err, "memory is not aligned");

        MutexRing* r = new (mem) MutexRing;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        int rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        if (rc == 0) rc = pthread_mutex_init(&r->mutex_, &attr);
        pthread_mutexattr_destroy(&attr);
        if (rc != 0) return fail(err, std::string("pthread mutex init: ") + std::strerror(rc));

        r->capacity_ = capacity;
        r->slotSize_ = sizeof(T);
        r->head_ = r->tail_ = r->count_ = 0;
        return r;
    }

    // Uses a ring another process already built in `mem` (`bytes` long). Checks that it was
    // built for the same slot type size and that its capacity fits, so a build with a different
    // message layout is refused instead of reading garbage.
    static MutexRing* attach(void* mem, size_t bytes, std::string* err) {
        if (reinterpret_cast<uintptr_t>(mem) % alignof(MutexRing) != 0) return fail(err, "memory is not aligned");
        if (bytes < sizeof(MutexRing)) return fail(err, "memory too small for a ring header");
        auto* r = std::launder(static_cast<MutexRing*>(mem));
        if (r->slotSize_ != sizeof(T))
            return fail(err, "ring slots are " + std::to_string(r->slotSize_) + " bytes, this build expects " +
                                 std::to_string(sizeof(T)));
        if (r->capacity_ == 0 || bytesFor(r->capacity_) != bytes)
            return fail(err, "ring capacity " + std::to_string(r->capacity_) + " does not match the memory size");
        return r;
    }

    // Call once, by the owner, when nobody uses the ring any more
    void destroy() { pthread_mutex_destroy(&mutex_); }

    // Producer side. false = ring full, nothing written.
    bool tryPush(const T& item) {
        pthread_mutex_lock(&mutex_);
        bool ok = count_ < capacity_;
        if (ok) {
            std::memcpy(slot(head_), &item, sizeof(T));
            head_ = (head_ + 1) % capacity_;
            ++count_;
        }
        pthread_mutex_unlock(&mutex_);
        return ok;
    }

    // Consumer side. false = ring empty, `out` untouched.
    bool tryPop(T& out) {
        pthread_mutex_lock(&mutex_);
        bool ok = count_ > 0;
        if (ok) {
            std::memcpy(&out, slot(tail_), sizeof(T));
            tail_ = (tail_ + 1) % capacity_;
            --count_;
        }
        pthread_mutex_unlock(&mutex_);
        return ok;
    }

    // Items waiting right now (for stats; may be stale by the time you read it)
    uint32_t size() {
        pthread_mutex_lock(&mutex_);
        uint32_t n = count_;
        pthread_mutex_unlock(&mutex_);
        return n;
    }

    uint32_t capacity() const { return capacity_; }

private:
    MutexRing() = default;

    static MutexRing* fail(std::string* err, std::string msg) {
        if (err) *err = std::move(msg);
        return nullptr;
    }

    // Slots start right after the header
    unsigned char* slot(uint32_t i) {
        return reinterpret_cast<unsigned char*>(this + 1) + size_t(i) * sizeof(T);
    }

    pthread_mutex_t mutex_;
    uint32_t capacity_;
    uint32_t slotSize_;  // sizeof(T) of the build that created it, checked by attach()
    uint32_t head_;   // next slot the producer writes
    uint32_t tail_;   // next slot the consumer reads
    uint32_t count_;  // items stored; full when == capacity_
};

}  // namespace lab
