// Ring buffer in shared memory, producer and consumer in two separate processes.
//
// Same checks as ring_test (every seq in order, content verified, tiny capacities that keep the
// ring full / empty and wrapping), but the consumer is a fork()ed process that opens the ring by
// name: its own mapping, at its own address. Also checks that a ring built for a different slot
// size is refused.

#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>

#include "args.hpp"
#include "cpu.hpp"
#include "messages.hpp"
#include "ring_check.hpp"
#include "ring_mutex.hpp"
#include "shm_ring.hpp"

using namespace lab;
using namespace lab::test;
using Ring = MutexRing<Msg>;

static const char* kName = "/lab_shm_ring_test";

// Consumer process. Exit code: 0 ok, 1 open failed, 2 wrong message, 3 pin failed.
static int consumer(uint64_t count, int cpu) {
    if (cpu >= 0 && !pinThread(cpu).empty()) return 3;
    std::string err;
    auto shm = ShmRing<Ring>::open(kName, &err);
    if (!shm) {
        std::printf("  consumer: %s\n", err.c_str());
        std::fflush(stdout);
        return 1;
    }
    Ring& ring = shm->ring();
    Msg m;
    for (uint64_t seq = 0; seq < count;) {
        if (!ring.tryPop(m)) continue;
        if (!matches(m, seq)) {
            std::printf("  consumer: MISMATCH at seq %llu, got seq %llu\n", (unsigned long long)seq,
                        (unsigned long long)m.hdr.seq);
            std::fflush(stdout);
            return 2;
        }
        ++seq;
    }
    return 0;
}

int main(int argc, char** argv) {
    Args args(argc, argv, "SPSC ring in shared memory: producer process -> consumer process");
    uint64_t count = args.get<uint64_t>("count", 2'000'000, "messages per capacity");
    int producerCpu = args.get<int>("producer-cpu", -1, "pin the producer process (-1 = don't pin)");
    int consumerCpu = args.get<int>("consumer-cpu", -1, "pin the consumer process (-1 = don't pin)");
    args.done();
    if (producerCpu >= 0) {
        std::string err = pinThread(producerCpu);
        if (!err.empty()) {
            std::fprintf(stderr, "pin failed: %s\n", err.c_str());
            return 1;
        }
    }

    int failures = 0;
    std::printf("producer = this process, consumer = a fork()ed process opening %s\n\n", kName);
    std::printf("  %8s %10s %12s  %s\n", "capacity", "messages", "msgs/s", "result");
    for (uint32_t capacity : {1u, 2u, 3u, 1024u}) {
        std::string err;
        auto shm = ShmRing<Ring>::create(kName, capacity, &err, true);
        if (!shm) {
            std::printf("  create failed: %s\n", err.c_str());
            return 1;
        }
        Ring& ring = shm->ring();

        auto start = std::chrono::steady_clock::now();
        std::fflush(stdout);
        pid_t pid = fork();
        if (pid == 0) _exit(consumer(count, consumerCpu));

        // If the consumer dies the ring stays full: check on it now and then instead of spinning for ever
        Msg m;
        int status = 0;
        bool consumerGone = false;
        uint64_t spins = 0;
        for (uint64_t seq = 0; seq < count && !consumerGone; ++seq) {
            fill(m, seq);
            while (!ring.tryPush(m))
                if (++spins % (1 << 20) == 0 && waitpid(pid, &status, WNOHANG) == pid) {
                    consumerGone = true;
                    break;
                }
        }
        if (!consumerGone) waitpid(pid, &status, 0);
        double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0 && ring.size() == 0;
        if (!ok) ++failures;
        std::printf("  %8u %10llu %12.0f  %s\n", capacity, (unsigned long long)count, double(count) / secs,
                    ok ? "ok" : ("FAILED, consumer exit " + std::to_string(WEXITSTATUS(status))).c_str());
    }  // owner's destructor unlinks the name

    // A build with a different message size must be refused
    struct Small {
        uint64_t x;
    };
    {
        std::string err;
        auto other = ShmRing<MutexRing<Small>>::create(kName, 16, &err, true);
        auto wrong = ShmRing<Ring>::open(kName, &err, 50);
        bool refused = other && !wrong;
        if (!refused) ++failures;
        std::printf("\n  ring of 8-byte slots opened as Msg ring: %s  %s\n", refused ? "refused, ok" : "FAILED",
                    err.c_str());
    }

    std::printf("\n%s\n", failures ? "FAILED" : "all shared memory ring checks passed");
    return failures ? 1 : 0;
}
