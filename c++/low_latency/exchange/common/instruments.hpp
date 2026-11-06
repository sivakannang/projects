#pragma once
// Instrument reference data shared by the order generator, the validator and the engine.

#include <cstdint>
#include <vector>

namespace lab {

struct Instrument {
    uint32_t id;
    int64_t refPrice;   // reference price (e.g. previous close), paise
    int64_t tickSize;   // paise
    uint32_t lotSize;   // quantity must be a multiple of this
    uint32_t maxQty;
    int64_t bandPct;    // limit prices must be within refPrice +/- bandPct %
};

// n futures-like contracts: prices 1000.00 .. rupees, tick 0.05, lots of 25/50/75/100
inline std::vector<Instrument> defaultInstruments(uint32_t n = 16) {
    static constexpr uint32_t kLots[] = {25, 50, 75, 100};
    std::vector<Instrument> out;
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t lot = kLots[i % 4];
        out.push_back({i, int64_t(1000 + 250 * i) * 100, 5, lot, lot * 100, 10});
    }
    return out;
}

}  // namespace lab
