/*
 *
 What problem does std::atomic solve?
   - When two threads access the same variable and at least one writes, you get a data race → undefined behavior.
   - std::atomic<T> lets you read/write/update a single object safely without a mutex, and (optionally) control visibility/ordering of memory effects between threads.
   - Think of it as: “a variable with built-in thread-safety and memory fences.”

   - Atomicity: operations happen as if indivisible (no torn reads/writes).
   - Ordering: you choose how operations are sequenced across threads (from “just atomic” to “strict global order”).
   - Lock-free (maybe): often lock-free on primitives; check with a.is_lock_free().

 Types you will use:
   - std::atomic<T> — for integral types, pointers, enums, some trivially copyable structs.
   - std::atomic_flag — tiny always lock-free boolean.
   - std::atomic_ref<T> (C++20) — atomic view on an existing object.
   - (C++20) wait/notify on atomics: a.wait(old), a.notify_one/all()

 Operations you will use most:
   - a.load(order);            // read
   - a.store(v, order);        // write
   - a.exchange(v, order);     // swap value, return old

   Read-Modify-Write (integral/pointer):
   - a.fetch_add(x); a.fetch_sub(x);
   - a.fetch_and(mask); a.fetch_or(mask); a.fetch_xor(mask);

   Compare-and-swap (CAS):
   - a.compare_exchange_weak(exp, desired, succ_order, fail_order);
   - a.compare_exchange_strong(exp, desired, succ_order, fail_order);

   C++20:
   - a.wait(old);              // sleep until a.load()!=old
   - a.notify_one/all();       // wake one/all waiter

| Order                | What it guarantees                            | Use for                           |
| -------------------- | --------------------------------------------- | --------------------------------- |
| `relaxed`            | Atomicity only (no visibility ordering)       | counters/metrics                  |
| `acquire` (on load)  | Later reads/writes can’t move **before** it   | reading a “ready” flag            |
| `release` (on store) | Earlier reads/writes can’t move **after** it  | publishing data then setting flag |
| `acq_rel`            | Both (for RMW ops)                            | `fetch_add`, successful CAS       |
| `seq_cst`            | Strongest, one global order across SC atomics | default when unsure               |


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
    constexpr int threads = 8;std::thread t(producer);
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

