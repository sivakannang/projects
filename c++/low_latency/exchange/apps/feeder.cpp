// feeder: Gateway simulator: sends orders at a configured rate, receives execution reports, measures latency.
//
// Pacing (--rate > 0): message i is due at a scheduled time and stamped with that time, not the
// moment it actually left. If the pipeline can't keep up, overdue messages are sent as soon as
// there is room (catch up), and their latency includes the wait: no coordinated omission.
//   --arrival=constant   due(i) = start + i / rate                (repeatable, for comparisons)
//   --arrival=poisson    random gaps with the same average rate   (many independent clients)
// --rate=0 sends as fast as the ring accepts.
//
// The flow is a pool of --pool orders generated up front and replayed in a loop. Each pass shifts
// every clientOrderId by the pool's highest id, so ids stay unique however long the run is.
//
// Every reply is checked: seq 0, 1, 2, ... in order, matching clientOrderId and status.
// One thread does both sending and collecting, so the pipeline needs one spinning thread per program.

#include <cmath>
#include <string>
#include <vector>

#include "order_gen.hpp"
#include "pipeline.hpp"
#include "rng.hpp"
#include "timing.hpp"

using namespace lab;

static uint64_t& requestId(Msg& m) {
    switch (m.hdr.type) {
        case MsgType::ModifyOrder: return m.modify.clientOrderId;
        case MsgType::CancelOrder: return m.cancel.clientOrderId;
        default: return m.newOrder.clientOrderId;
    }
}

static ExecStatus expectedStatus(MsgType t) {
    switch (t) {
        case MsgType::NewOrder: return ExecStatus::Accepted;
        case MsgType::ModifyOrder: return ExecStatus::Modified;
        case MsgType::CancelOrder: return ExecStatus::Cancelled;
        default: return ExecStatus::Rejected;
    }
}

// Message seq of the endless replay: pool entry seq % size, ids shifted by one offset per pass
class Replay {
public:
    Replay(std::vector<Msg> pool, uint64_t idOffset) : pool_(std::move(pool)), offset_(idOffset) {}

    void build(uint64_t seq, Msg& m) const {
        m = pool_[seq % pool_.size()];
        requestId(m) += (seq / pool_.size()) * offset_;
        m.hdr.seq = seq;
    }
    uint64_t expectedId(uint64_t seq) const {
        Msg m = pool_[seq % pool_.size()];
        return requestId(m) + (seq / pool_.size()) * offset_;
    }
    MsgType type(uint64_t seq) const { return pool_[seq % pool_.size()].hdr.type; }

private:
    std::vector<Msg> pool_;
    uint64_t offset_;
};

int main(int argc, char** argv) {
    Args args(argc, argv, "Gateway simulator: sends orders at a set rate, receives execution reports, checks them");
    StageOptions opt = stageOptions(args);
    double rate = args.get<double>("rate", 0, "orders per second (0 = as fast as possible)");
    double duration = args.get<double>("duration", 0, "seconds to send for (0 = send --count orders)");
    uint64_t count = args.get<uint64_t>("count", 1'000'000, "orders to send when --duration is 0");
    std::string arrival = args.get<std::string>("arrival", "constant", "constant or poisson gaps between orders");
    uint64_t poolSize = args.get<uint64_t>("pool", 1'000'000, "orders generated up front and replayed in a loop");
    uint64_t seed = args.get<uint64_t>("seed", 42, "order flow seed");
    bool progress = args.flag("progress", "log one line per second (each costs a few us of send loop time)");
    args.done();
    bool poisson = arrival == "poisson";
    if ((arrival != "constant" && !poisson) || rate < 0 || duration < 0 || poolSize == 0 || (!duration && !count)) {
        std::fprintf(stderr, "feeder: bad option values (run --help)\n");
        return 2;
    }

    startProcess("feeder", opt.cpu);
    calibrate();

    // How much to send: a count, rate x duration, or (unpaced with a duration) until time is up
    uint64_t total = duration == 0 ? count : rate > 0 ? uint64_t(rate * duration) : UINT64_MAX;
    uint64_t deadlineTicks = (duration > 0 && rate == 0) ? nsToTicks(duration * 1e9) : 0;

    GenConfig gen;
    gen.seed = seed;
    OrderGenerator generator(gen);
    Replay replay(generator.generate(std::min(poolSize, total)), generator.lastOrderId());
    if (rate > 0)
        LOG_INFO("sending %llu orders at %.0f/s (%s), pool %llu", (unsigned long long)total, rate, arrival.c_str(),
                 (unsigned long long)std::min(poolSize, total));
    else if (deadlineTicks)
        LOG_INFO("sending as fast as possible for %.1f s, pool %llu", duration, (unsigned long long)poolSize);
    else
        LOG_INFO("sending %llu orders as fast as possible", (unsigned long long)total);

    std::unique_ptr<PipeRing> in, out;
    if (!connectRings(opt, kSenderToFeeder, kFeederToReceiver, in, out)) return 1;
    installStopHandler();
    auto& replies = in->ring();
    auto& orders = out->ring();

    // Wait until the whole loop is connected: a Ping must come all the way round
    {
        Msg ping{};
        ping.hdr.type = MsgType::Ping;
        while (!orders.tryPush(ping) && !stopRequested()) {
        }
        uint64_t sentAt = now();
        Msg r;
        while (!stopRequested() && !(replies.tryPop(r) && r.hdr.type == MsgType::Ping)) {
        }
        LOG_INFO("all stages connected (ping round trip %.1f us), starting", ticksToNs(now() - sentAt) / 1e3);
    }

    double interval = rate > 0 ? ticksPerNs() * 1e9 / rate : 0;  // average gap in ticks
    Rng gaps(seed ^ 0x9a95);                                       // Poisson gaps, separate from the flow
    auto nextGap = [&] { return poisson ? -std::log(1.0 - gaps.unit()) * interval : interval; };

    uint64_t sent = 0, received = 0, errors = 0, fullSpins = 0, emptySpins = 0, maxLag = 0;
    bool sentEnd = false, gotEnd = false, built = false;
    auto report = [&](const char* what, uint64_t seq) {
        if (++errors <= 5) LOG_ERROR("reply %llu: %s", (unsigned long long)seq, what);
    };

    Msg m;
    uint64_t start = now(), lastSend = start;
    double due = double(start);  // scheduled time of message `sent`
    uint64_t oneSecond = nsToTicks(1e9), nextProgress = start + oneSecond, lastSent = 0, lastReceived = 0;

    while (!gotEnd && !stopRequested()) {
        uint64_t t = now();

        // Send: the next order if it is due (and the ring has room), or EndOfStream when done
        if (!sentEnd) {
            bool more = sent < total && !(deadlineTicks && t - start >= deadlineTicks);
            if (!more) {
                Msg end{};
                end.hdr.type = MsgType::EndOfStream;
                end.hdr.seq = sent;
                if (orders.tryPush(end)) sentEnd = true;
            } else if (rate == 0 || double(t) >= due) {
                if (!built) {
                    replay.build(sent, m);
                    built = true;
                }
                m.hdr.tsFeeder = rate > 0 ? uint64_t(due) : t;
                if (orders.tryPush(m)) {
                    if (rate > 0 && t - uint64_t(due) > maxLag) maxLag = t - uint64_t(due);
                    ++sent;
                    built = false;
                    lastSend = t;
                    due += nextGap();
                } else {
                    ++fullSpins;
                }
            }
        }

        // Collect one reply if there is one
        Msg r;
        if (replies.tryPop(r)) {
            if (r.hdr.type == MsgType::EndOfStream) {
                gotEnd = true;
            } else {
                if (r.hdr.seq != received || received >= sent)
                    report("out of sequence", r.hdr.seq);
                else if (r.hdr.type != MsgType::ExecReport)
                    report("not an ExecReport", r.hdr.seq);
                else if (r.exec.clientOrderId != replay.expectedId(received))
                    report("clientOrderId does not match the order sent", r.hdr.seq);
                else if (r.exec.status != expectedStatus(replay.type(received)))
                    report("unexpected status", r.hdr.seq);
                ++received;
            }
        } else {
            ++emptySpins;
        }

        if (progress && t >= nextProgress) {
            double behindMs = rate > 0 && double(t) > due ? ticksToNs(uint64_t(double(t) - due)) / 1e6 : 0;
            LOG_INFO("sent %llu/s, replies %llu/s, in flight %llu, behind schedule %.2f ms",
                     (unsigned long long)(sent - lastSent), (unsigned long long)(received - lastReceived),
                     (unsigned long long)(sent - received), behindMs);
            lastSent = sent;
            lastReceived = received;
            nextProgress += oneSecond;
        }
    }
    uint64_t end = now();

    double sendSecs = ticksToSec(lastSend - start);
    double achieved = sendSecs > 0 ? double(sent) / sendSecs : 0;
    bool ok = gotEnd && received == sent && errors == 0;
    LOG_INFO("%s: sent %llu, received %llu, errors %llu, %.3f s", gotEnd ? "end of stream" : "stopped",
             (unsigned long long)sent, (unsigned long long)received, (unsigned long long)errors,
             ticksToSec(end - start));
    if (rate > 0) {
        LOG_INFO("rate: target %.0f/s, achieved %.0f/s, max %.3f ms behind schedule", rate, achieved,
                 ticksToNs(maxLag) / 1e6);
        if (achieved < rate * 0.99)
            LOG_WARN("the pipeline could not keep up with %.0f/s", rate);
        else if (ticksToNs(maxLag) > 1e6)
            LOG_INFO("kept up on average, but fell behind for a while (a stall, or a burst of arrivals)");
    } else {
        LOG_INFO("rate: %.0f/s, as fast as possible", achieved);
    }
    LOG_INFO("output full %llu times, input empty %llu times", (unsigned long long)fullSpins,
             (unsigned long long)emptySpins);
    if (ok)
        LOG_INFO("every message made the round trip in order");
    else if (!gotEnd)
        LOG_WARN("stopped before all replies came back");
    else
        LOG_ERROR("PIPELINE CHECK FAILED");
    return ok ? 0 : 1;
}
