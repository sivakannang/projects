#pragma once
// Small, fast, deterministic random numbers (splitmix64).
//
// Not std::mt19937 + std::uniform_int_distribution: the standard distributions are
// implemented differently by libc++ (Mac) and libstdc++ (Linux), so the same seed gives
// different streams on different machines. Our own generator + our own range mapping
// gives the identical stream everywhere.

#include <cstdint>

namespace lab {

class Rng {
public:
    explicit Rng(uint64_t seed) : state_(seed) {}

    uint64_t next() {
        uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    // Uniform in [0, n): multiply-shift instead of %, no division (bias is negligible here)
    uint32_t below(uint32_t n) { return uint32_t((uint64_t(uint32_t(next() >> 32)) * n) >> 32); }

    // Uniform in [0, 1), 53 random bits
    double unit() { return double(next() >> 11) * 0x1.0p-53; }

    // True with the given probability in percent
    bool chance(uint32_t pct) { return below(100) < pct; }

private:
    uint64_t state_;
};

}  // namespace lab
