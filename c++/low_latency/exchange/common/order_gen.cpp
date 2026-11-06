#include "order_gen.hpp"

#include <algorithm>

namespace lab {

std::string GenConfig::check() const {
    if (newPct + modifyPct + cancelPct != 100) return "new + modify + cancel percentages must add up to 100";
    if (iocPct + marketPct > 100) return "ioc + market percentages must be <= 100";
    if (instruments == 0 || accounts == 0 || maxLots == 0) return "instruments, accounts and max lots must be > 0";
    if (depthTicks == 0 || aggressiveTicks == 0) return "depth and aggressive ticks must be > 0";
    if (hotInstruments > instruments) return "hot instruments can't exceed instruments";
    return {};
}

OrderGenerator::OrderGenerator(const GenConfig& cfg)
    : cfg_(cfg), rng_(cfg.seed), instruments_(defaultInstruments(cfg.instruments)) {
    live_.reserve(kMaxLive);
}

std::vector<Msg> OrderGenerator::generate(size_t n) {
    std::vector<Msg> out(n);
    for (Msg& m : out) next(m);
    return out;
}

void OrderGenerator::next(Msg& m) {
    m = Msg{};
    uint32_t roll = rng_.below(100);
    if (roll < cfg_.cancelPct && !live_.empty()) makeCancel(m);
    else if (roll < cfg_.cancelPct + cfg_.modifyPct && !live_.empty()) makeModify(m);
    else makeNew(m);  // also used while there is nothing to modify or cancel yet
}

void OrderGenerator::makeNew(Msg& m) {
    m.hdr.type = MsgType::NewOrder;
    NewOrder& o = m.newOrder;
    const Instrument& ins = instruments_[pickInstrument()];

    o.clientOrderId = ++lastId_;
    o.instrument = ins.id;
    o.account = 1 + rng_.below(cfg_.accounts);
    o.side = rng_.below(2) ? Side::Buy : Side::Sell;
    o.qty = ins.lotSize * (1 + rng_.below(cfg_.maxLots));

    uint32_t kind = rng_.below(100);
    if (kind < cfg_.marketPct) {
        o.ordType = OrdType::Market;  // never rests: fills what it can within protection, rest expires
        o.tif = Tif::Ioc;
        o.price = 0;
    } else if (kind < cfg_.marketPct + cfg_.iocPct) {
        o.ordType = OrdType::Limit;
        o.tif = Tif::Ioc;
        o.price = aggressivePrice(ins, o.side);  // IOC is meant to trade now
    } else {
        o.ordType = OrdType::Limit;
        o.tif = Tif::Day;
        o.price = rng_.chance(cfg_.aggressivePct) ? aggressivePrice(ins, o.side) : passivePrice(ins, o.side);
    }

    if (rng_.chance(cfg_.invalidPct)) corrupt(o, ins);
    else if (o.tif == Tif::Day) remember(o);
}

void OrderGenerator::makeModify(Msg& m) {
    Live& l = live_[rng_.below(uint32_t(live_.size()))];
    const Instrument& ins = instruments_[l.instrument];
    m.hdr.type = MsgType::ModifyOrder;
    ModifyOrder& mo = m.modify;
    mo.clientOrderId = l.id;
    mo.instrument = l.instrument;
    mo.account = l.account;

    uint32_t kind = rng_.below(100);
    uint32_t lots = l.qty / ins.lotSize;
    if (kind < 50 && lots > 1) {
        l.qty -= ins.lotSize * (1 + rng_.below(lots - 1));  // qty down: keeps time priority
    } else if (kind < 75) {
        l.qty = std::min(l.qty + ins.lotSize * (1 + rng_.below(3)), ins.maxQty);  // qty up: loses priority
    } else {
        l.price = passivePrice(ins, l.side);  // new price: loses priority
    }
    mo.newPrice = l.price;
    mo.newQty = l.qty;
}

void OrderGenerator::makeCancel(Msg& m) {
    size_t i = rng_.below(uint32_t(live_.size()));
    const Live& l = live_[i];
    m.hdr.type = MsgType::CancelOrder;
    m.cancel.clientOrderId = l.id;
    m.cancel.instrument = l.instrument;
    m.cancel.account = l.account;
    live_[i] = live_.back();  // never cancel the same order twice
    live_.pop_back();
}

// Exactly one validation error, cycling through the validator's checks
void OrderGenerator::corrupt(NewOrder& o, const Instrument& ins) {
    switch (rng_.below(7)) {
        case 0: o.qty = 0; break;                                    // BadQty
        case 1: o.qty += 1; break;                                   // BadLot
        case 2: o.qty = ins.maxQty + ins.lotSize; break;             // BadQty (too large)
        case 3: o.price = ins.refPrice * 2; o.ordType = OrdType::Limit; break;  // PriceBand
        case 4: o.price = ins.refPrice + 1; o.ordType = OrdType::Limit; break;  // BadTick
        case 5: o.instrument = cfg_.instruments + 7; break;          // BadInstrument
        case 6: o.account = 0; break;                                // BadAccount
    }
}

void OrderGenerator::remember(const NewOrder& o) {
    Live l{o.clientOrderId, o.price, o.qty, o.instrument, o.account, o.side};
    if (live_.size() < kMaxLive) live_.push_back(l);
    else live_[rng_.below(uint32_t(kMaxLive))] = l;
}

uint32_t OrderGenerator::pickInstrument() {
    uint32_t n = cfg_.instruments, hot = cfg_.hotInstruments;
    if (hot == 0 || hot == n) return rng_.below(n);
    return rng_.chance(cfg_.hotPct) ? rng_.below(hot) : hot + rng_.below(n - hot);
}

// Buy below / sell above the reference price: rests without crossing
int64_t OrderGenerator::passivePrice(const Instrument& ins, Side side) {
    int64_t ticks = 1 + rng_.below(cfg_.depthTicks);
    return ins.refPrice + (side == Side::Buy ? -ticks : ticks) * ins.tickSize;
}

// Buy above / sell below the reference price: crosses resting orders on the other side
int64_t OrderGenerator::aggressivePrice(const Instrument& ins, Side side) {
    int64_t ticks = 1 + rng_.below(cfg_.aggressiveTicks);
    return ins.refPrice + (side == Side::Buy ? ticks : -ticks) * ins.tickSize;
}

}  // namespace lab
