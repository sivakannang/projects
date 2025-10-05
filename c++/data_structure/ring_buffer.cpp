#include <array>
#include <cstddef>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <semaphore>

// ========================= 1) Simple (single-threaded) =========================
template<typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0, "N must be > 0");
private:
    std::array<T, N> buf_{};
    std::size_t head_ = 0; // next write
    std::size_t tail_ = 0; // next read
    std::size_t count_ = 0;
public:
    bool push(const T& v) {
        if (count_ == N) return false;                 // full
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        ++count_;
        return true;
    }
    bool pop(T& out) {
        if (count_ == 0) return false;                 // empty
        out = buf_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;
        return true;
    }
    bool empty() const { return count_ == 0; }
    bool full()  const { return count_ == N; }
    std::size_t size() const { return count_; }
    static constexpr std::size_t capacity() { return N; }
};

// ==================== 2) Thread-safe (MPMC, blocking) =========================

template<typename T, std::size_t N>
class TSRingBuffer {
    static_assert(N > 0, "N must be > 0");
private:
    std::array<T, N> buf_{};
    std::size_t head_ = 0; // next write
    std::size_t tail_ = 0; // next read
    std::size_t count_ = 0;

    mutable std::mutex m_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
public:
    // Blocking push
    void push(const T& v) {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_full_.wait(lk, [&]{ return count_ < N; });
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        ++count_;
        lk.unlock();
        cv_not_empty_.notify_one();
    }
    // Blocking pop
    void pop(T& out) {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_empty_.wait(lk, [&]{ return count_ > 0; });
        out = buf_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;
        lk.unlock();
        cv_not_full_.notify_one();
    }
    // Non-blocking variants (minimal)
    bool try_push(const T& v) {
        std::lock_guard<std::mutex> g(m_);
        if (count_ == N) return false;
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        ++count_;
        cv_not_empty_.notify_one();
        return true;
    }
    bool try_pop(T& out) {
        std::lock_guard<std::mutex> g(m_);
        if (count_ == 0) return false;
        out = buf_[tail_];
        tail_ = (tail_ + 1) % N;
        --count_;
        cv_not_full_.notify_one();
        return true;
    }
    bool empty() const {
        std::lock_guard<std::mutex> g(m_);
        return count_ == 0;
    }
    bool full() const {
        std::lock_guard<std::mutex> g(m_);
        return count_ == N;
    }
    std::size_t size() const {
        std::lock_guard<std::mutex> g(m_);
        return count_;
    }
    static constexpr std::size_t capacity() { return N; }
};

// ====================== 3) Lock-free SPSC (non-blocking) ======================

template<typename T, std::size_t N>
class LFRingBuffer {
    static_assert(N > 0, "N must be > 0");
    std::array<T, N> buf_{};
    std::atomic<std::size_t> head_{0}; // producer: next write (unbounded)
    std::atomic<std::size_t> tail_{0}; // consumer: next read  (unbounded)
public:
    // Producer thread
    bool try_push(const T& v) {
        std::size_t h = head_.load(std::memory_order_relaxed);
        std::size_t t = tail_.load(std::memory_order_acquire);
        if (h - t == N) return false;         // full
        buf_[h % N] = v;                      // index only is modulo N
        head_.store(h + 1, std::memory_order_release);
        return true;
    }
    // Consumer thread
    bool try_pop(T& out) {
        std::size_t t = tail_.load(std::memory_order_relaxed);
        std::size_t h = head_.load(std::memory_order_acquire);
        if (h == t) return false;             // empty
        out = buf_[t % N];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        std::size_t t = tail_.load(std::memory_order_acquire);
        std::size_t h = head_.load(std::memory_order_acquire);
        return h == t;
    }
    bool full() const {
        std::size_t h = head_.load(std::memory_order_acquire);
        std::size_t t = tail_.load(std::memory_order_acquire);
        return h - t == N;
    }
    std::size_t size() const {                // approximate under concurrency (SPSC-safe)
        std::size_t h = head_.load(std::memory_order_acquire);
        std::size_t t = tail_.load(std::memory_order_acquire);
        return h - t;
    }

    static constexpr std::size_t capacity() { return N; }
};


template<class T, std::size_t N>
class SpscRing {
    static_assert(N > 0, "Capacity N must be > 0");

    std::array<T, N> buf_{};
    std::size_t head_ = 0;                 // next pop index
    std::size_t tail_ = 0;                 // next push index

    std::counting_semaphore<N> slots_{N};   // free slots
    std::counting_semaphore<N> items_{0};  // ready items

public:
    // blocking push/pop
    void push(const T& v) {
        slots_.acquire();
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        items_.release();
    }
    void push(T&& v) {
        slots_.acquire();
        buf_[head_] = std::move(v);
        head_ = (head_ + 1) % N;
        items_.release();
    }
    T pop() {
        items_.acquire();
        T v = std::move(buf_[tail_]);
        tail_ = (tail_ + 1) % N;
        slots_.release();
        return v;
    }

    // non-blocking tries
    bool try_push(const T& v) {
        if (!slots_.try_acquire()) return false;
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        items_.release();
        return true;
    }
    bool try_pop(T& out) {
        if (!items_.try_acquire()) return false;
        out = std::move(buf_[tail_]);
        tail_ = (tail_ + 1) % N;
        slots_.release();
        return true;
    }
};


void demo_semaphore () {
    SpscRing<int, 8> q;

    std::jthread prod([&]{
        for (int i = 1; i <= 10; ++i) q.push(i);
    });
    std::jthread cons([&]{
        for (int i = 1; i <= 10; ++i) std::cout << q.pop() << ' ';
    });
}




using namespace std::chrono_literals;

// -------------------- Simple (single-thread) demo --------------------
void demo_RingBuffer() {
    RingBuffer<int, 4> rb;
    for (int i = 1; i <= 4; ++i) {
        std::cout << "push " << i << " -> " << std::boolalpha << rb.push(i) << '\n';
    }
    std::cout << "full? " << rb.full() << ", size=" << rb.size() << '\n';
    std::cout << "push 99 (should fail) -> " << rb.push(99) << '\n';

    int x;
    while (rb.pop(x)) {
        std::cout << "pop " << x << '\n';
    }
    std::cout << "empty? " << rb.empty() << ", size=" << rb.size() << "\n\n";
}

// -------------------- Thread-safe (MPMC, blocking) demo --------------------
void demo_TSRingBuffer() {
    TSRingBuffer<int, 8> rb;

    constexpr int producers = 2;
    constexpr int consumers = 2;
    constexpr int items_per_producer = 10;
    const int total_items = producers * items_per_producer;

    std::atomic<int> received{0};
    std::atomic<long long> sum{0};

    // Producers
    std::vector<std::thread> P;
    for (int p = 0; p < producers; ++p) {
        P.emplace_back([&, p] {
            for (int i = 1; i <= items_per_producer; ++i) {
                rb.push(p * 100 + i); // identifiable payloads: 101..110, 201..210
            }
        });
    }

    // Consumers
    std::vector<std::thread> C;
    for (int c = 0; c < consumers; ++c) {
        C.emplace_back([&] {
            for (;;) {
                int v;
                rb.pop(v);                 // blocking
                if (v == -1) break;        // sentinel to stop
                sum.fetch_add(v, std::memory_order_relaxed);
                received.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : P) t.join();

    // Push one sentinel per consumer to let them exit cleanly.
    for (int i = 0; i < consumers; ++i) rb.push(-1);

    for (auto& t : C) t.join();

    std::cout << "TSRingBuffer: received=" << received.load()
              << "/" << total_items
              << ", queue empty? " << std::boolalpha << rb.empty()
              << ", sum=" << sum.load() << "\n\n";
}

// -------------------- Lock-free SPSC demo --------------------
void demo_LFRingBuffer() {
    LFRingBuffer<int, 8> rb;   // Use power-of-two capacity to be compatible with either variant
    constexpr int total = 20;

    std::thread producer([&] {
        for (int i = 1; i <= total; ++i) {
            while (!rb.try_push(i)) {
                std::this_thread::yield(); // backoff
            }
        }
    });

    std::atomic<int> got{0};
    std::thread consumer([&] {
        int v;
        while (got.load() < total) {
            if (rb.try_pop(v)) {
                // Uncomment to see the stream:
                // std::cout << "lf pop " << v << '\n';
                got.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    std::cout << "LFRingBuffer: got=" << got.load()
              << "/" << total
              << ", empty? " << std::boolalpha << rb.empty()
              << ", size=" << rb.size() << "\n\n";
}

int main() {
    std::cout << "--- RingBuffer (single-thread) ---\n";
    demo_RingBuffer();

    std::cout << "--- TSRingBuffer (MPMC, blocking) ---\n";
    demo_TSRingBuffer();

    std::cout << "--- LFRingBuffer (SPSC, lock-free) ---\n";
    demo_LFRingBuffer();

    return 0;
}

