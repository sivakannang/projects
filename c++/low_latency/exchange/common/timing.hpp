#pragma once
// Cheap timestamps for latency measurement.
//
// x86: now() is the rdtsc instruction (~7-10 ns). It counts "ticks" of the time-stamp counter.
//      On CPUs with an invariant TSC (constant_tsc + nonstop_tsc in /proc/cpuinfo) the counter
//      runs at a fixed rate whatever the current CPU speed, and all cores share it, so a tick
//      taken on one core can be subtracted from a tick taken on another (or in another process).
// Other CPUs (Apple ARM): now() is steady_clock in nanoseconds, so 1 tick = 1 ns.
//
// Ticks are converted to ns with a factor measured once against steady_clock (calibrate()).
// Call calibrate() at startup, never inside a measured loop: it sleeps ~200 ms.
//
//   lab::calibrate();
//   uint64_t t0 = lab::now();  ...work...  double ns = lab::ticksToNs(lab::now() - t0);

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>

#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#include <x86intrin.h>
#define LAB_HAVE_RDTSC 1
#endif

namespace lab {

inline uint64_t now() {
#ifdef LAB_HAVE_RDTSC
    // Plain rdtsc: the CPU may execute it a few instructions early or late. That costs a few ns
    // of precision, irrelevant for the hundreds of ns we measure. (rdtscp / lfence would fix it.)
    return __rdtsc();
#else
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count());
#endif
}

namespace detail {
inline double& ticksPerNsRef() {
    static double v = 0;
    return v;
}
}  // namespace detail

// Measures ticks per ns against steady_clock over `ms` milliseconds. Returns the factor.
inline double calibrate(int ms = 200) {
#ifdef LAB_HAVE_RDTSC
    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();
    uint64_t c0 = now();
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    auto t1 = clock::now();
    uint64_t c1 = now();
    double ns = double(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    detail::ticksPerNsRef() = double(c1 - c0) / ns;
#else
    (void)ms;
    detail::ticksPerNsRef() = 1.0;
#endif
    return detail::ticksPerNsRef();
}

// Factor from the last calibrate() (calibrates on first use if nobody did)
inline double ticksPerNs() {
    if (detail::ticksPerNsRef() == 0) calibrate();
    return detail::ticksPerNsRef();
}

inline double ticksToNs(uint64_t ticks) { return double(ticks) / ticksPerNs(); }
inline double ticksToSec(uint64_t ticks) { return ticksToNs(ticks) * 1e-9; }
inline uint64_t nsToTicks(double ns) { return uint64_t(ns * ticksPerNs()); }

// True if the TSC runs at a constant rate in all power states (CPUID 0x80000007, EDX bit 8)
inline bool invariantTsc() {
#ifdef LAB_HAVE_RDTSC
    unsigned a, b, c, d;
    if (!__get_cpuid(0x80000007, &a, &b, &c, &d)) return false;
    return (d >> 8) & 1;
#else
    return false;
#endif
}

// One line for benchmark reports, e.g. "rdtsc, 1.696 ticks/ns, invariant TSC"
inline std::string clockDescription() {
    char buf[96];
#ifdef LAB_HAVE_RDTSC
    std::snprintf(buf, sizeof buf, "rdtsc, %.3f ticks/ns, %s", ticksPerNs(),
                  invariantTsc() ? "invariant TSC" : "TSC NOT invariant (cross-core deltas unreliable)");
#else
    std::snprintf(buf, sizeof buf, "steady_clock, 1 tick = 1 ns");
#endif
    return buf;
}

}  // namespace lab
