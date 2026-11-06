/*
 * Low-latency micro-benchmarks
 * ----------------------------
 * A tiny benchmark harness + runnable demos for the classic low-latency problems
 * listed in profiling.cpp (cache misses, false sharing, branch misprediction,
 * virtual dispatch, malloc, tail latency).
 *
 * Build : g++ -std=c++20 -O2 -march=native -pthread benchmark.cpp -o bench
 *         (Apple clang: drop -march=native if it complains, use -mcpu=native)
 * Run   : ./bench
 *
 * Benchmarking rules
 *   1. Always build with optimisation (-O2/-O3). -O0 numbers are meaningless.
 *   2. Warm up first (caches, branch predictor, page faults, CPU frequency).
 *   3. Repeat and report min / median, not a single run.
 *   4. Stop the compiler deleting the work -> doNotOptimize() / clobberMemory().
 *   5. For latency, record every sample and report percentiles (p50/p99/p99.9/max),
 *      the average hides the tail.
 *   6. Use a monotonic clock (steady_clock), never system_clock.
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <new>
#include <random>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------

// Tells the compiler the value is used, so the computation can't be optimised away
template <typename T>
inline void doNotOptimize(T const& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

// Forces pending writes to be treated as visible (compiler barrier only, no CPU fence)
inline void clobberMemory() {
    asm volatile("" : : : "memory");
}

inline int64_t nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
}

// Runs fn() `reps` times after `warmup` runs, prints min and median wall time per run
template <typename Fn>
double bench(const char* name, Fn&& fn, int reps = 15, int warmup = 3) {
    for (int i = 0; i < warmup; ++i) fn();

    std::vector<double> samples;
    samples.reserve(reps);
    for (int i = 0; i < reps; ++i) {
        int64_t start = nowNs();
        fn();
        clobberMemory();
        samples.push_back((nowNs() - start) / 1e6);
    }
    std::sort(samples.begin(), samples.end());
    std::printf("  %-34s min %8.3f ms   median %8.3f ms\n", name, samples.front(), samples[reps / 2]);
    return samples[reps / 2];
}

// Latency histogram: per-operation samples -> percentiles
struct LatencyStats {
    std::vector<int64_t> ns;

    void report(const char* name) {
        std::sort(ns.begin(), ns.end());
        auto pct = [&](double p) { return ns[std::min(ns.size() - 1, size_t(p * ns.size()))]; };
        double avg = 0;
        for (auto v : ns) avg += v;
        avg /= ns.size();
        std::printf("  %-22s avg %6.0f  p50 %6lld  p99 %6lld  p99.9 %7lld  max %8lld  (ns)\n",
                    name, avg, (long long)pct(0.50), (long long)pct(0.99), (long long)pct(0.999),
                    (long long)ns.back());
    }
};

// ---------------------------------------------------------------------------
// 1. Cache locality: vector vs list
// ---------------------------------------------------------------------------
void benchCacheLocality() {
    std::puts("\n[1] Cache locality - contiguous vs node based traversal");
    constexpr int N = 2'000'000;

    std::vector<int> vec(N);
    std::list<int> lst;
    std::mt19937 rng(42);
    for (int i = 0; i < N; ++i) {
        vec[i] = int(rng() & 0xff);
        lst.push_back(vec[i]);
    }

    bench("std::vector sum", [&] {
        long sum = 0;
        for (int v : vec) sum += v;
        doNotOptimize(sum);
    });
    bench("std::list sum", [&] {
        long sum = 0;
        for (int v : lst) sum += v;
        doNotOptimize(sum);
    });
}

// ---------------------------------------------------------------------------
// 2. False sharing
// ---------------------------------------------------------------------------
#ifdef __cpp_lib_hardware_interference_size
constexpr size_t kCacheLine = std::hardware_destructive_interference_size;
#else
constexpr size_t kCacheLine = 128;  // 64 on most x86, 128 on Apple M-series; 128 is safe for both
#endif

struct SharedCounters {
    std::atomic<long> a{0};
    std::atomic<long> b{0};  // same cache line as `a`
};

struct PaddedCounters {
    alignas(kCacheLine) std::atomic<long> a{0};
    alignas(kCacheLine) std::atomic<long> b{0};  // own cache line
};

template <typename Counters>
void hammer(Counters& c) {
    constexpr long kIters = 20'000'000;
    std::thread t1([&] { for (long i = 0; i < kIters; ++i) c.a.fetch_add(1, std::memory_order_relaxed); });
    std::thread t2([&] { for (long i = 0; i < kIters; ++i) c.b.fetch_add(1, std::memory_order_relaxed); });
    t1.join();
    t2.join();
}

void benchFalseSharing() {
    std::puts("\n[2] False sharing - two threads, two independent counters");
    bench("same cache line", [] { SharedCounters c; hammer(c); }, 5, 1);
    bench("padded (alignas cache line)", [] { PaddedCounters c; hammer(c); }, 5, 1);
}

// ---------------------------------------------------------------------------
// 3. Branch prediction
// ---------------------------------------------------------------------------
void benchBranchPrediction() {
    std::puts("\n[3] Branch prediction - if (x >= 128) on random vs sorted data");
    constexpr int N = 1 << 22;
    std::vector<int> random(N);
    std::mt19937 rng(7);
    for (auto& v : random) v = int(rng() % 256);
    std::vector<int> sorted = random;
    std::sort(sorted.begin(), sorted.end());

    // Note: at -O2/-O3 compilers often turn this into a branchless cmov/SIMD select,
    // which removes the difference. noinline + the volatile-ish accumulate keeps the branch.
    auto branchy = [](const std::vector<int>& d) {
        long sum = 0;
        for (int v : d) {
            if (v >= 128) {
                sum += v;
                clobberMemory();
            }
        }
        doNotOptimize(sum);
    };
    auto branchless = [](const std::vector<int>& d) {
        long sum = 0;
        for (int v : d) sum += v & -(v >= 128);  // mask instead of jump
        doNotOptimize(sum);
    };

    bench("branchy, random data", [&] { branchy(random); });
    bench("branchy, sorted data", [&] { branchy(sorted); });
    bench("branchless, random data", [&] { branchless(random); });
}

// ---------------------------------------------------------------------------
// 4. Virtual dispatch vs CRTP (static polymorphism)
// ---------------------------------------------------------------------------
struct ShapeV {
    virtual ~ShapeV() = default;
    virtual long area(long x) const = 0;
};
struct SquareV : ShapeV {
    long area(long x) const override { return x * x; }
};

template <typename Derived>
struct ShapeC {
    long area(long x) const { return static_cast<const Derived*>(this)->areaImpl(x); }
};
struct SquareC : ShapeC<SquareC> {
    long areaImpl(long x) const { return x * x; }
};

// Input comes from memory so the compiler can't fold the loop into a closed-form formula
__attribute__((noinline)) long sumVirtual(const ShapeV& s, const std::vector<long>& in) {
    long sum = 0;
    for (long x : in) sum += s.area(x);
    return sum;
}

template <typename S>
__attribute__((noinline)) long sumCrtp(const ShapeC<S>& s, const std::vector<long>& in) {
    long sum = 0;
    for (long x : in) sum += s.area(x);  // inlined -> vectorisable
    return sum;
}

void benchDispatch() {
    std::puts("\n[4] Dynamic vs static dispatch");
    constexpr long N = 10'000'000;
    std::vector<long> in(N);
    for (long i = 0; i < N; ++i) in[i] = i & 0xffff;
    // Hide the concrete type from the optimiser so it can't devirtualise
    std::unique_ptr<ShapeV> v(std::rand() >= 0 ? new SquareV : nullptr);
    SquareC c;
    bench("virtual call", [&] { doNotOptimize(sumVirtual(*v, in)); }, 7, 1);
    bench("CRTP (inlined)", [&] { doNotOptimize(sumCrtp(c, in)); }, 7, 1);
}

// ---------------------------------------------------------------------------
// 5. malloc vs pre-allocated pool + allocation tail latency
// ---------------------------------------------------------------------------
struct Order {
    uint64_t id;
    double price;
    uint32_t qty;
    char side;
};

// Fixed size, single-threaded free-list pool: O(1), no syscalls, no locks
template <typename T, size_t Capacity>
class ObjectPool {
    union Slot {
        Slot* next;
        alignas(T) unsigned char storage[sizeof(T)];
    };
    std::unique_ptr<Slot[]> slots_{new Slot[Capacity]};
    Slot* free_ = nullptr;

public:
    ObjectPool() {
        for (size_t i = 0; i < Capacity; ++i) {
            slots_[i].next = free_;
            free_ = &slots_[i];
        }
    }
    template <typename... Args>
    T* create(Args&&... args) {
        Slot* s = free_;
        free_ = s->next;
        return new (s->storage) T{std::forward<Args>(args)...};
    }
    void destroy(T* p) {
        p->~T();
        Slot* s = reinterpret_cast<Slot*>(p);
        s->next = free_;
        free_ = s;
    }
};

void benchAllocation() {
    std::puts("\n[5] Allocation - new/delete vs object pool");
    constexpr int N = 1'000'000;
    static ObjectPool<Order, N> pool;
    std::vector<Order*> ptrs(N);

    bench("new / delete", [&] {
        for (int i = 0; i < N; ++i) ptrs[i] = new Order{uint64_t(i), 100.5, 10, 'B'};
        for (int i = 0; i < N; ++i) delete ptrs[i];
    }, 7, 2);
    bench("object pool", [&] {
        for (int i = 0; i < N; ++i) ptrs[i] = pool.create(uint64_t(i), 100.5, 10u, 'B');
        for (int i = 0; i < N; ++i) pool.destroy(ptrs[i]);
    }, 7, 2);

    // Per-operation latency: the average looks fine, the tail tells the truth.
    // On Apple Silicon steady_clock ticks at 24 MHz (~42 ns), so single ops read as 0 or 42;
    // on x86 use rdtsc/rdtscp for cycle-level resolution.
    std::puts("\n[6] Tail latency - per allocation (timer overhead included)");
    constexpr int S = 200'000;
    LatencyStats heap, pooled;
    heap.ns.reserve(S);
    pooled.ns.reserve(S);
    for (int i = 0; i < S; ++i) {
        int64_t t0 = nowNs();
        ptrs[i] = new Order{uint64_t(i), 1.0, 1, 'S'};
        int64_t t1 = nowNs();
        heap.ns.push_back(t1 - t0);
    }
    for (int i = 0; i < S; ++i) delete ptrs[i];
    for (int i = 0; i < S; ++i) {
        int64_t t0 = nowNs();
        ptrs[i] = pool.create(uint64_t(i), 1.0, 1u, 'S');
        int64_t t1 = nowNs();
        pooled.ns.push_back(t1 - t0);
    }
    for (int i = 0; i < S; ++i) pool.destroy(ptrs[i]);
    heap.report("new");
    pooled.report("pool");

    LatencyStats timer;
    timer.ns.reserve(S);
    for (int i = 0; i < S; ++i) {
        int64_t t0 = nowNs();
        int64_t t1 = nowNs();
        timer.ns.push_back(t1 - t0);
    }
    timer.report("empty (clock cost)");
}

int main() {
    std::printf("cache line used for padding: %zu bytes, hw threads: %u\n", kCacheLine,
                std::thread::hardware_concurrency());
    benchCacheLocality();
    benchFalseSharing();
    benchBranchPrediction();
    benchDispatch();
    benchAllocation();
    return 0;
}
