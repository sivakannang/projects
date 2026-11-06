#pragma once
// Pieces shared by the pipeline programs: ring names, Ctrl-C handling, and the stage loop.
//
//   feeder ──/lab_f2r──▶ receiver ──/lab_r2p──▶ processor ──/lab_p2s──▶ sender
//     ▲                                                                   │
//     └─────────────────────────────/lab_s2f──────────────────────────────┘
//
// Ring ownership: every program creates the ring it READS (its mailbox) and opens the ring it
// writes, waiting for that one to appear. So the start order doesn't matter, and each program
// unlinks its own ring when it exits.
//
// Shutdown: the feeder sends MsgType::EndOfStream after its last order. Each stage passes it on
// and exits; it can't overtake real messages, so nothing in flight is lost. Ctrl-C / SIGTERM
// sets a flag that every loop checks, so an interrupted run still cleans up its rings.
//
// Start: the feeder sends one MsgType::Ping round the loop and starts its clock only when it
// comes back, so paced runs don't start while a later stage is still connecting.

#include <signal.h>

#include <atomic>
#include <string>

#include "args.hpp"
#include "log.hpp"
#include "messages.hpp"
#include "process.hpp"
#include "ring_mutex.hpp"
#include "shm_ring.hpp"

namespace lab {

using PipeRing = ShmRing<MutexRing<Msg>>;

inline constexpr const char* kFeederToReceiver = "/lab_f2r";
inline constexpr const char* kReceiverToProcessor = "/lab_r2p";
inline constexpr const char* kProcessorToSender = "/lab_p2s";
inline constexpr const char* kSenderToFeeder = "/lab_s2f";

// ---- Ctrl-C ------------------------------------------------------------------------------

inline std::atomic<bool> gStop{false};

inline void installStopHandler() {
    struct sigaction sa{};
    sa.sa_handler = [](int) { gStop.store(true, std::memory_order_relaxed); };
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

inline bool stopRequested() { return gStop.load(std::memory_order_relaxed); }

// ---- options every pipeline program has --------------------------------------------------

struct StageOptions {
    int cpu;
    uint32_t capacity;  // of the ring this program creates (its input)
    int waitMs;         // how long to wait for the output ring to appear
};

inline StageOptions stageOptions(Args& args) {
    StageOptions o;
    o.cpu = args.get<int>("cpu", -1, "CPU to pin this process to (-1 = don't pin)");
    o.capacity = args.get<uint32_t>("capacity", 1024, "capacity of this program's input ring");
    o.waitMs = args.get<int>("wait-ms", 30000, "how long to wait for the next program's ring");
    return o;
}

// Creates this program's input ring and opens its output ring. Logs and returns false on failure.
inline bool connectRings(const StageOptions& opt, const char* inName, const char* outName,
                         std::unique_ptr<PipeRing>& in, std::unique_ptr<PipeRing>& out) {
    std::string err;
    in = PipeRing::create(inName, opt.capacity, &err, true);  // replace: a crashed run may have left it
    if (!in) {
        LOG_ERROR("%s", err.c_str());
        return false;
    }
    LOG_INFO("created %s (capacity %u), waiting for %s", inName, opt.capacity, outName);
    out = PipeRing::open(outName, &err, opt.waitMs);
    if (!out) {
        LOG_ERROR("%s", err.c_str());
        return false;
    }
    LOG_INFO("connected: reading %s, writing %s", inName, outName);
    return true;
}

// ---- the pass-through stage loop (receiver, processor, sender) ---------------------------

// Pops from the input ring, checks the sequence, calls work(msg), pushes to the output ring,
// until EndOfStream (passed on) or Ctrl-C. Returns the process exit code.
template <typename Work>
int runStage(const char* name, const StageOptions& opt, const char* inName, const char* outName, Work work) {
    startProcess(name, opt.cpu);
    std::unique_ptr<PipeRing> in, out;
    if (!connectRings(opt, inName, outName, in, out)) return 1;
    installStopHandler();  // after connecting: Ctrl-C while still waiting just kills the process
    auto& input = in->ring();
    auto& output = out->ring();

    uint64_t handled = 0, gaps = 0, emptySpins = 0, fullSpins = 0, expected = 0;
    bool endOfStream = false;
    Msg m;
    while (!stopRequested()) {
        if (!input.tryPop(m)) {
            ++emptySpins;
            continue;
        }
        if (m.hdr.type == MsgType::EndOfStream) {
            endOfStream = true;
        } else if (m.hdr.type == MsgType::Ping) {
            // control message: pass on untouched, not part of the order sequence
        } else {
            if (m.hdr.seq != expected) ++gaps;
            expected = m.hdr.seq + 1;
            work(m);
            ++handled;
        }
        while (!output.tryPush(m) && !stopRequested()) ++fullSpins;
        if (endOfStream) break;
    }

    LOG_INFO("%s: handled %llu, seq gaps %llu, input empty %llu times, output full %llu times",
             endOfStream ? "end of stream" : "stopped", (unsigned long long)handled, (unsigned long long)gaps,
             (unsigned long long)emptySpins, (unsigned long long)fullSpins);
    return gaps == 0 ? 0 : 1;
}

}  // namespace lab
