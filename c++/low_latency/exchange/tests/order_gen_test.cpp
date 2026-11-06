// Order generator test: shows sample messages, the actual mix vs the configured one,
// checks determinism (same seed -> same checksum) and measures generation cost.
//
// The checksum printed at the end must be identical on every machine for the same seed.

#include <chrono>
#include <cmath>
#include <cstdio>

#include "args.hpp"
#include "msg_format.hpp"
#include "order_gen.hpp"

using namespace lab;

static int failures = 0;

// FNV-1a over every byte of every message (pads included, so they must be deterministic too)
static uint64_t checksum(const std::vector<Msg>& flow) {
    uint64_t h = 0xcbf29ce484222325ULL;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(flow.data());
    for (size_t i = 0; i < flow.size() * sizeof(Msg); ++i) h = (h ^ p[i]) * 0x100000001b3ULL;
    return h;
}

static bool isInvalid(const NewOrder& o, const std::vector<Instrument>& ins) {
    if (o.instrument >= ins.size() || o.account == 0) return true;
    const Instrument& r = ins[o.instrument];
    if (o.qty == 0 || o.qty > r.maxQty || o.qty % r.lotSize) return true;
    if (o.ordType == OrdType::Market) return false;
    int64_t band = r.refPrice * r.bandPct / 100;
    return o.price % r.tickSize || o.price < r.refPrice - band || o.price > r.refPrice + band;
}

static void expect(const char* what, double actual, double configured, double tolerance) {
    bool ok = std::fabs(actual - configured) <= tolerance;
    std::printf("  %-28s %6.2f %%   (configured %5.1f %%)  %s\n", what, actual, configured, ok ? "ok" : "OFF");
    if (!ok) ++failures;
}

int main(int argc, char** argv) {
    Args args(argc, argv, "Order generator: sample, mix, determinism and speed");
    size_t count = args.get<size_t>("count", 1'000'000, "messages to generate");
    uint64_t seed = args.get<uint64_t>("seed", 42, "random seed");
    int dump = args.get<int>("dump", 20, "how many messages to print");
    args.done();

    GenConfig cfg;
    cfg.seed = seed;
    if (std::string err = cfg.check(); !err.empty()) {
        std::printf("bad config: %s\n", err.c_str());
        return 1;
    }

    OrderGenerator gen(cfg);
    auto t0 = std::chrono::steady_clock::now();
    std::vector<Msg> flow = gen.generate(count);
    auto t1 = std::chrono::steady_clock::now();
    double nsPerMsg = std::chrono::duration<double, std::nano>(t1 - t0).count() / double(count);

    std::printf("First %d messages (seed %llu):\n", dump, (unsigned long long)seed);
    for (int i = 0; i < dump && size_t(i) < flow.size(); ++i) std::printf("  %s\n", formatMsg(flow[i]).c_str());

    // ---- actual mix ----
    size_t news = 0, modifies = 0, cancels = 0, market = 0, ioc = 0, day = 0, aggressiveDay = 0, invalid = 0, hot = 0;
    const auto& ins = gen.instruments();
    for (const Msg& m : flow) {
        uint32_t instrument = 0;
        switch (m.hdr.type) {
            case MsgType::NewOrder: {
                const NewOrder& o = m.newOrder;
                ++news;
                instrument = o.instrument;
                if (isInvalid(o, ins)) { ++invalid; break; }
                if (o.ordType == OrdType::Market) ++market;
                else if (o.tif == Tif::Ioc) ++ioc;
                else {
                    ++day;
                    int64_t ref = ins[o.instrument].refPrice;
                    if ((o.side == Side::Buy && o.price > ref) || (o.side == Side::Sell && o.price < ref)) ++aggressiveDay;
                }
                break;
            }
            case MsgType::ModifyOrder: ++modifies; instrument = m.modify.instrument; break;
            case MsgType::CancelOrder: ++cancels; instrument = m.cancel.instrument; break;
            default: break;
        }
        if (instrument < cfg.hotInstruments) ++hot;
    }
    auto pct = [](size_t part, size_t whole) { return whole ? 100.0 * double(part) / double(whole) : 0.0; };
    size_t validNew = news - invalid;

    std::printf("\nMix over %zu messages:\n", count);
    expect("New", pct(news, count), cfg.newPct, 0.5);
    expect("Modify", pct(modifies, count), cfg.modifyPct, 0.5);
    expect("Cancel", pct(cancels, count), cfg.cancelPct, 0.5);
    expect("Invalid (of new)", pct(invalid, news), cfg.invalidPct, 0.3);
    expect("Market (of valid new)", pct(market, validNew), cfg.marketPct, 0.5);
    expect("IOC (of valid new)", pct(ioc, validNew), cfg.iocPct, 0.5);
    expect("Aggressive (of Limit Day)", pct(aggressiveDay, day), cfg.aggressivePct, 0.5);
    expect("Hot instruments (of all)", pct(hot, count), cfg.hotPct, 1.0);

    // ---- determinism ----
    uint64_t sum = checksum(flow);
    uint64_t again = checksum(OrderGenerator(cfg).generate(count));
    GenConfig other = cfg;
    other.seed = seed + 1;
    uint64_t different = checksum(OrderGenerator(other).generate(count));
    std::printf("\nDeterminism:\n");
    std::printf("  checksum seed %llu        %016llx\n", (unsigned long long)seed, (unsigned long long)sum);
    std::printf("  same seed again          %016llx  %s\n", (unsigned long long)again, again == sum ? "ok" : "MISMATCH");
    std::printf("  seed %llu (should differ) %016llx  %s\n", (unsigned long long)(seed + 1),
                (unsigned long long)different, different != sum ? "ok" : "SAME");
    if (again != sum || different == sum) ++failures;

    std::printf("\nSpeed: %.1f ns per message (%.1f M msgs/s), flow size %.1f MB\n", nsPerMsg, 1e3 / nsPerMsg,
                double(count * sizeof(Msg)) / 1e6);
    std::printf("\n%s\n", failures ? "FAILED" : "all order generator checks passed");
    return failures ? 1 : 0;
}
