// Ring buffer correctness test: one producer thread, one consumer thread.
//
// The producer pushes messages with seq = 0, 1, 2, ... and content derived from seq.
// The consumer checks every message is exactly the next seq with the expected content:
// that proves no loss, no duplicates, no reordering and no corruption.
// Tiny capacities (1, 2, 3) keep the ring constantly full / empty and wrapping, which is
// where ring bugs hide. The msgs/s figure is only a rough indication; task 7 measures properly.

#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>

#include "args.hpp"
#include "cpu.hpp"
#include "messages.hpp"
#include "ring_check.hpp"
#include "ring_mutex.hpp"

using namespace lab;
using namespace lab::test;
using Ring = MutexRing<Msg>;

struct Result {
    bool ok = true;
    uint64_t received = 0;
    uint64_t fullSpins = 0;   // producer found the ring full
    uint64_t emptySpins = 0;  // consumer found the ring empty
    double seconds = 0;
};

static Result run(uint32_t capacity, uint64_t count, int producerCpu, int consumerCpu) {
    Result res;
    std::string err;
    void* mem = ::operator new(Ring::bytesFor(capacity), std::align_val_t(64));
    Ring* ring = Ring::create(mem, capacity, &err);
    if (!ring) {
        std::printf("  create failed: %s\n", err.c_str());
        res.ok = false;
        return res;
    }

    auto start = std::chrono::steady_clock::now();

    std::thread producer([&] {
        setThreadName("producer");
        if (producerCpu >= 0) pinThread(producerCpu);
        Msg m;
        for (uint64_t seq = 0; seq < count; ++seq) {
            fill(m, seq);
            while (!ring->tryPush(m)) ++res.fullSpins;  // baseline waiting: just retry
        }
    });

    std::thread consumer([&] {
        setThreadName("consumer");
        if (consumerCpu >= 0) pinThread(consumerCpu);
        Msg m;
        while (res.received < count) {
            if (!ring->tryPop(m)) {
                ++res.emptySpins;
                continue;
            }
            if (res.ok && !matches(m, res.received)) {
                std::printf("  MISMATCH: expected seq %llu, got seq %llu\n", (unsigned long long)res.received,
                            (unsigned long long)m.hdr.seq);
                res.ok = false;
            }
            ++res.received;
        }
    });

    producer.join();
    consumer.join();
    res.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    if (ring->size() != 0) {
        std::printf("  ring not empty at the end: %u items\n", ring->size());
        res.ok = false;
    }
    ring->destroy();
    ::operator delete(mem, std::align_val_t(64));
    return res;
}

int main(int argc, char** argv) {
    Args args(argc, argv, "SPSC ring buffer test: no loss, no duplicates, no reordering");
    uint64_t count = args.get<uint64_t>("count", 2'000'000, "messages per capacity");
    int producerCpu = args.get<int>("producer-cpu", -1, "pin the producer thread (-1 = don't pin)");
    int consumerCpu = args.get<int>("consumer-cpu", -1, "pin the consumer thread (-1 = don't pin)");
    args.done();

    std::printf("MutexRing<Msg>: header %zu bytes, slot %zu bytes\n\n", sizeof(Ring), sizeof(Msg));
    std::printf("  %8s %10s %12s %14s %14s  %s\n", "capacity", "messages", "msgs/s", "full spins", "empty spins",
                "result");

    int failures = 0;
    for (uint32_t capacity : {1u, 2u, 3u, 1024u}) {
        Result r = run(capacity, count, producerCpu, consumerCpu);
        if (!r.ok || r.received != count) ++failures;
        std::printf("  %8u %10llu %12.0f %14llu %14llu  %s\n", capacity, (unsigned long long)r.received,
                    double(r.received) / r.seconds, (unsigned long long)r.fullSpins,
                    (unsigned long long)r.emptySpins, r.ok ? "ok" : "FAILED");
    }

    std::printf("\n%s\n", failures ? "FAILED" : "all ring checks passed");
    return failures ? 1 : 0;
}
