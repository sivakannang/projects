#pragma once
// Deterministic order flow generator: same seed -> identical stream on every machine.
//
// The feeder generates the whole flow up front and then replays it, so generation cost
// never shows up in the send loop (same idea as replaying captured production data).
// The generator fills the message type and body; the feeder fills seq and timestamps.

#include <string>
#include <vector>

#include "instruments.hpp"
#include "messages.hpp"
#include "rng.hpp"

namespace lab {

struct GenConfig {
    uint64_t seed = 42;
    uint32_t instruments = 16;
    uint32_t accounts = 1000;

    // Message mix, percent of all messages (must add up to 100)
    uint32_t newPct = 70;
    uint32_t modifyPct = 10;
    uint32_t cancelPct = 20;

    // New order mix, percent of new orders; the rest are Limit Day
    uint32_t iocPct = 15;
    uint32_t marketPct = 5;
    uint32_t aggressivePct = 30;  // share of Limit Day orders priced to cross immediately
    uint32_t invalidPct = 2;      // share of new orders with one deliberate validation error

    // Skew: hotPct % of the flow goes to the first hotInstruments instruments
    uint32_t hotInstruments = 2;
    uint32_t hotPct = 50;

    // Prices: passive orders rest 1..depthTicks away from the reference price,
    // aggressive ones cross by 1..aggressiveTicks
    uint32_t depthTicks = 10;
    uint32_t aggressiveTicks = 3;
    uint32_t maxLots = 10;

    // "" if the settings make sense, otherwise what is wrong
    std::string check() const;
};

class OrderGenerator {
public:
    explicit OrderGenerator(const GenConfig& cfg);

    void next(Msg& m);
    std::vector<Msg> generate(size_t n);

    const std::vector<Instrument>& instruments() const { return instruments_; }
    uint64_t lastOrderId() const { return lastId_; }  // the feeder uses it to keep ids unique on replay

private:
    // A Limit Day order we created, which may still be resting: target for Modify / Cancel
    struct Live {
        uint64_t id;
        int64_t price;
        uint32_t qty;
        uint32_t instrument;
        uint32_t account;
        Side side;
    };

    void makeNew(Msg& m);
    void makeModify(Msg& m);
    void makeCancel(Msg& m);
    void corrupt(NewOrder& o, const Instrument& ins);
    void remember(const NewOrder& o);
    uint32_t pickInstrument();
    int64_t passivePrice(const Instrument& ins, Side side);
    int64_t aggressivePrice(const Instrument& ins, Side side);

    static constexpr size_t kMaxLive = 8192;

    GenConfig cfg_;
    Rng rng_;
    std::vector<Instrument> instruments_;
    std::vector<Live> live_;
    uint64_t lastId_ = 0;
};

}  // namespace lab
