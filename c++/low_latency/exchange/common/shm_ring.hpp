#pragma once
// A ring buffer living in a named shared memory segment.
//
//   /dev/shm/lab_feed:  [ ShmHeader 64 B ][ ring header | slot 0 | slot 1 | ... ]
//
// The owner creates it (ring built in place, then marked ready); other processes open it by name
// and attach to the existing ring without re-initialising it. The owner's destructor unlinks the
// name; everybody else only unmaps.
//
//   auto feed = lab::ShmRing<lab::MutexRing<lab::Msg>>::create("/lab_feed", 1024, &err);   // owner
//   auto feed = lab::ShmRing<lab::MutexRing<lab::Msg>>::open("/lab_feed", &err);           // other side
//   feed->ring().tryPush(msg);
//
// Ring must provide bytesFor(capacity), create(mem, capacity, err) and attach(mem, bytes, err).

#include <memory>
#include <string>

#include "shm.hpp"

namespace lab {

template <typename Ring>
class ShmRing {
public:
    static std::unique_ptr<ShmRing> create(const std::string& name, uint32_t capacity, std::string* err,
                                           bool replace = false) {
        auto shm = SharedMemory::create(name, Ring::bytesFor(capacity), err, replace);
        if (!shm) return nullptr;
        Ring* ring = Ring::create(shm->payload(), capacity, err);
        if (!ring) {
            SharedMemory::unlink(name);
            return nullptr;
        }
        shm->markReady();
        return std::unique_ptr<ShmRing>(new ShmRing(std::move(shm), ring, true));
    }

    static std::unique_ptr<ShmRing> open(const std::string& name, std::string* err, int timeoutMs = 2000) {
        auto shm = SharedMemory::open(name, err, timeoutMs);
        if (!shm) return nullptr;
        Ring* ring = Ring::attach(shm->payload(), shm->payloadSize(), err);
        if (!ring) {
            if (err) *err = name + ": " + *err;
            return nullptr;
        }
        return std::unique_ptr<ShmRing>(new ShmRing(std::move(shm), ring, false));
    }

    // Owner: remove the name. The ring itself is not torn down, other processes may still use it.
    ~ShmRing() {
        if (owner_) SharedMemory::unlink(shm_->name());
    }
    ShmRing(const ShmRing&) = delete;
    ShmRing& operator=(const ShmRing&) = delete;

    Ring& ring() { return *ring_; }
    const std::string& name() const { return shm_->name(); }
    bool owner() const { return owner_; }

private:
    ShmRing(std::unique_ptr<SharedMemory> shm, Ring* ring, bool owner)
        : shm_(std::move(shm)), ring_(ring), owner_(owner) {}

    std::unique_ptr<SharedMemory> shm_;
    Ring* ring_;
    bool owner_;
};

}  // namespace lab
