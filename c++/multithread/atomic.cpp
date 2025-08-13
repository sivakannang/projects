// atomic_showcase.cpp
// Build: g++ -std=gnu++20 -O2 -Wall -Wextra -pthread atomic_showcase.cpp -o atomic_demo

/*
==========================
std::atomic — Notes
==========================
- What: Atomic ops on a single object to prevent data races without a mutex.
- Default memory order: seq_cst (strongest). You can pass weaker orders per operation.
- Lock-free? Check obj.is_lock_free() and atomic<T>::is_always_lock_free (platform + alignment dependent).
- Types: atomic<T> works for trivially copyable T (lock-free not guaranteed). Not copy-assignable—use load/store/CAS.
- volatile != atomic: volatile is for MMIO; it does not synchronize threads.
- Memory orders (practical):
  * relaxed: atomicity only (counters/metrics)
  * acquire (loads): later ops can’t move before
  * release (stores): earlier ops can’t move after
  * acq_rel: for read-modify-write ops
  * seq_cst: strongest, globally ordered
  * consume: effectively unusable; treat as acquire

- Common patterns:
  * Publish data: write data → store flag (release); reader loads flag (acquire) → sees data.
  * CAS loop: modify based on current value until compare-exchange succeeds.
  * Atomic flag spinlock: only for tiny critical sections; otherwise prefer std::mutex.
  * C++20 wait/notify: futex-like blocking waits on atomics (intra-process).

==========================
Interview Questions (crisp answers)
==========================
1) relaxed use case? → stats/counters where only atomicity matters; no inter-thread ordering needed.
2) acquire vs release? → acquire on loads prevents later ops from moving before; release on stores prevents earlier ops from moving after.
3) weak vs strong CAS? → weak may spuriously fail (use in a loop); strong reduces spurious failures.
4) Why volatile isn’t for threads? → doesn’t establish happens-before; no atomicity or ordering guarantees.
5) Publish pointer + data? → write data, then store(ptr, release); reader load(acquire) → sees initialized data.
6) When mutex over atomics? → multi-step invariants/critical sections; clarity over micro-optimizations.
7) atomic_ref? → C++20 view providing atomic ops on an existing properly aligned object.
*/

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

#define PRINT(sec) std::cout << "\n=== " << sec << " ===\n"

// ------------------------------------------------------------
// 1) Atomic counter (relaxed) — many threads, one counter
// ------------------------------------------------------------
void demo_atomic_counter() {
    PRINT("Atomic counter (relaxed)");
    std::atomic<unsigned> counter{0};
    constexpr int threads = 8;
    constexpr int iters   = 250'000;

    std::vector<std::thread> th;
    th.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        th.emplace_back([&]{
            for (int k = 0; k < iters; ++k)
                counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : th) t.join();

    std::cout << "expected: " << (threads * iters)
              << ", got: "   << counter.load() << "\n";
}

// ------------------------------------------------------------
// 2) Publish data with release/acquire
// ------------------------------------------------------------
struct Payload { int a{0}, b{0}; };

void demo_release_acquire() {
    PRINT("Publish with release/acquire");
    Payload data{};
    std::atomic<bool> ready{false};

    std::thread producer([&]{
        data.a = 10;
        data.b = 20;
        ready.store(true, std::memory_order_release);  // publish
    });

    std::thread consumer([&]{
        while (!ready.load(std::memory_order_acquire)) { /* spin briefly */ }
        std::cout << "seen: a=" << data.a << " b=" << data.b << "\n";
        assert(data.a == 10 && data.b == 20);
    });

    producer.join();
    consumer.join();
    std::cout << "OK (acquire saw the published data)\n";
}

// ------------------------------------------------------------
// 3) CAS loop on double — portable add without fetch_add(double)
// ------------------------------------------------------------
std::atomic<double> g_total{0.0};

void add_double(double x) {
    double old = g_total.load(std::memory_order_relaxed);
    while (!g_total.compare_exchange_weak(old, old + x,
                                          std::memory_order_acq_rel,
                                          std::memory_order_relaxed)) {
        // 'old' updated with the current value on failure
    }
}

void demo_cas_double() {
    PRINT("CAS loop on double");
    g_total.store(0.0, std::memory_order_relaxed);

    std::thread t1([]{ for (int i=0;i<100'000;++i) add_double(0.01); });
    std::thread t2([]{ for (int i=0;i<100'000;++i) add_double(0.02); });
    t1.join(); t2.join();

    double v = g_total.load(std::memory_order_relaxed);
    std::cout.setf(std::ios::fixed); std::cout.precision(2);
    std::cout << "total ≈ " << v << " (expected ≈ " << (100000*0.01 + 100000*0.02) << ")\n";
}

// ------------------------------------------------------------
// 4) Tiny spinlock with atomic_flag (use sparingly)
// ------------------------------------------------------------
struct SpinLock {
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    void lock()   { while (f.test_and_set(std::memory_order_acquire)) { /* spin */ } }
    void unlock() { f.clear(std::memory_order_release); }
};

void demo_spinlock() {
    PRINT("Spinlock (atomic_flag)");
    SpinLock sl;
    int shared = 0;

    auto worker = [&]{
        for (int i = 0; i < 100'000; ++i) {
            sl.lock();
            ++shared;
            sl.unlock();
        }
    };

    std::thread t1(worker), t2(worker), t3(worker), t4(worker);
    t1.join(); t2.join(); t3.join(); t4.join();

    std::cout << "shared = " << shared << " (expected 400000)\n";
}

// ------------------------------------------------------------
// 5) C++20 wait/notify on atomics (intra-process)
// ------------------------------------------------------------
void demo_wait_notify() {
    PRINT("C++20 atomic wait/notify");
    std::atomic<int> state{0};      // start at 0

    std::thread waiter([&]{
        int expected = 0;
        std::cout << "waiter: waiting for state != 0 ...\n";
        state.wait(expected);       // blocks until state != expected
        std::cout << "waiter: woke, state=" << state.load() << "\n";
    });

    std::this_thread::sleep_for(50ms);
    std::thread notifier([&]{
        state.store(1, std::memory_order_release);
        state.notify_one();
        std::cout << "notifier: notified\n";
    });

    waiter.join();
    notifier.join();
}

// ------------------------------------------------------------
// 6) atomic_ref — atomics on an existing object
// ------------------------------------------------------------
alignas(8) std::uint64_t raw_counter = 0;

void demo_atomic_ref() {
    PRINT("atomic_ref on existing object");
    std::atomic_ref<std::uint64_t> ref(raw_counter); // requires proper alignment

    auto bump = [&]{
        for (int i = 0; i < 200'000; ++i)
            ref.fetch_add(1, std::memory_order_relaxed);
    };

    std::thread t1(bump), t2(bump), t3(bump);
    t1.join(); t2.join(); t3.join();

    std::cout << "raw_counter = " << raw_counter << " (expected 600000)\n";
}

// ------------------------------------------------------------
int main() {
    demo_atomic_counter();
    demo_release_acquire();
    demo_cas_double();
    demo_spinlock();
    demo_wait_notify();
    demo_atomic_ref();

    std::cout << "\nAll demos done.\n";
    return 0;
}

