#pragma once
// Latency samples and percentiles.
//
// Every sample is stored in a buffer allocated up front, so recording is one store and never
// allocates inside the measured loop. After the run the samples are sorted and percentiles read
// off exactly. (A fixed-size histogram replaces this later for continuous monitoring, task 15.)
//
//   lab::Samples s(1'000'000);
//   s.add(ticks);  ...
//   lab::Percentiles p = s.percentiles(10);   // drop the first 10% as warm-up, values in ns

#include <algorithm>
#include <cstdint>
#include <vector>

#include "timing.hpp"

namespace lab {

struct Percentiles {
    uint64_t count = 0;  // samples used (after warm-up)
    double mean = 0, p50 = 0, p90 = 0, p99 = 0, p999 = 0, p9999 = 0, max = 0;  // ns
};

class Samples {
public:
    explicit Samples(size_t capacity) : v_(capacity) {}

    // Extra samples beyond the capacity are dropped (counted in dropped())
    void add(uint64_t ticks) {
        if (n_ < v_.size())
            v_[n_++] = ticks;
        else
            ++dropped_;
    }

    size_t size() const { return n_; }
    uint64_t dropped() const { return dropped_; }
    void clear() { n_ = dropped_ = 0; }

    // Sorts a copy of the samples after skipping the first warmupPct %, in recording order
    Percentiles percentiles(double warmupPct = 0) const {
        size_t skip = size_t(double(n_) * warmupPct / 100.0);
        std::vector<uint64_t> s(v_.begin() + skip, v_.begin() + n_);
        Percentiles p;
        p.count = s.size();
        if (s.empty()) return p;
        std::sort(s.begin(), s.end());
        double sum = 0;
        for (uint64_t x : s) sum += double(x);
        auto at = [&](double q) { return ticksToNs(s[std::min(s.size() - 1, size_t(q * double(s.size())))]); };
        p.mean = ticksToNs(uint64_t(sum / double(s.size())));
        p.p50 = at(0.50);
        p.p90 = at(0.90);
        p.p99 = at(0.99);
        p.p999 = at(0.999);
        p.p9999 = at(0.9999);
        p.max = ticksToNs(s.back());
        return p;
    }

private:
    std::vector<uint64_t> v_;
    size_t n_ = 0;
    uint64_t dropped_ = 0;
};

}  // namespace lab
