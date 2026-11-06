// Ring buffer benchmark: throughput and latency, measured separately.
//
// Throughput: the producer pushes N messages as fast as it can; msgs/s = N / time.
//             One warm-up run, then --repeats runs; the median is the result.
//
// Latency:    the producer sends at a fixed rate, below the maximum: --loads are percentages of
//             the measured median throughput (or one absolute --rate). Message i is scheduled at
//             start + i * interval. The consumer computes latency = pop time - send stamp.
//
//             Each message carries two stamps, to show coordinated omission:
//               scheduled: when the message should have been sent (hdr.tsFeeder). If the producer
//                          is held up, the delay counts against every message that was due:
//                          what a real client would see. This is the correct number.
//               actual:    when the producer really got round to sending it. Delays before that
//                          moment are invisible, so stalls vanish from the results.
//
// Where the ring lives and who runs the two sides (one change at a time):
//   default       threads, ring in ordinary heap memory
//   --shm         threads, ring in a shared memory segment
//   --processes   two fork()ed processes, each opening the shared memory ring by name
//
// Rules: nothing is allocated or printed inside measured loops, the first --warmup % of latency
// samples is discarded, and the environment is printed with every result.

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "args.hpp"
#include "cpu.hpp"
#include "messages.hpp"
#include "ring_mutex.hpp"
#include "shm_ring.hpp"
#include "stats.hpp"
#include "timing.hpp"

#ifndef LAB_BUILD
#define LAB_BUILD "?"
#endif

using namespace lab;
using Ring = MutexRing<Msg>;

static const char* kRingName = "/lab_ring_bench";

enum class Mode { Heap, Shm, Processes };

static const char* modeName(Mode m) {
    switch (m) {
        case Mode::Heap: return "threads-heap";
        case Mode::Shm: return "threads-shm";
        case Mode::Processes: return "processes-shm";
    }
    return "?";
}

struct Config {
    uint32_t capacity;
    int producerCpu;
    int consumerCpu;
    Mode mode;
};

static void die(const std::string& msg) {
    std::fprintf(stderr, "%s\n", msg.c_str());
    std::exit(1);
}

static void pinOrDie(int cpu) {
    if (cpu < 0) return;
    std::string err = pinThread(cpu);
    if (!err.empty()) die("pin to CPU " + std::to_string(cpu) + " failed: " + err);
}

// Both sides set themselves up, then wait here so neither starts before the other is ready
struct StartGate {
    std::atomic<int> ready{0};
    std::atomic<bool> go{false};

    void arriveAndWait() {
        ready.fetch_add(1);
        while (!go.load()) {
        }
    }
    void open(int sides) {
        while (ready.load() < sides) {
        }
        go.store(true);
    }
};

// What the two sides report back. Lives in MAP_SHARED memory so fork()ed processes can write it
// (the atomics in StartGate are lock-free, so they work across processes too).
struct Shared {
    StartGate gate;
    uint64_t start = 0;   // producer: first message
    uint64_t end = 0;     // consumer: last message
    uint64_t maxLag = 0;  // producer: furthest behind schedule, ticks
    Percentiles scheduled, actual;
};

class SharedBlock {
public:
    SharedBlock() {
        mem_ = mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (mem_ == MAP_FAILED) die("mmap of the results block failed");
        sh_ = new (mem_) Shared;
    }
    ~SharedBlock() { munmap(mem_, sizeof(Shared)); }
    Shared& operator*() { return *sh_; }

private:
    void* mem_;
    Shared* sh_;
};

// A heap ring in its own aligned block, freed on scope exit
class HeapRing {
public:
    explicit HeapRing(uint32_t capacity) {
        mem_ = ::operator new(Ring::bytesFor(capacity), std::align_val_t(64));
        std::string err;
        ring_ = Ring::create(mem_, capacity, &err);
        if (!ring_) die("ring create failed: " + err);
    }
    ~HeapRing() {
        ring_->destroy();
        ::operator delete(mem_, std::align_val_t(64));
    }
    Ring& ring() { return *ring_; }

private:
    void* mem_;
    Ring* ring_;
};

// Runs producer(ring, shared) and consumer(ring, shared) on a fresh ring, as two threads or two
// processes. Each side calls shared.gate.arriveAndWait() itself once its setup is done.
template <typename Producer, typename Consumer>
static void runPair(const Config& cfg, Shared& sh, Producer producer, Consumer consumer) {
    if (cfg.mode == Mode::Heap || cfg.mode == Mode::Shm) {
        std::unique_ptr<HeapRing> heap;
        std::unique_ptr<ShmRing<Ring>> shm;
        std::string err;
        Ring* ring;
        if (cfg.mode == Mode::Heap) {
            heap = std::make_unique<HeapRing>(cfg.capacity);
            ring = &heap->ring();
        } else {
            shm = ShmRing<Ring>::create(kRingName, cfg.capacity, &err, true);
            if (!shm) die(err);
            ring = &shm->ring();
        }
        std::thread p([&] {
            setThreadName("producer");
            pinOrDie(cfg.producerCpu);
            producer(*ring, sh);
        });
        std::thread c([&] {
            setThreadName("consumer");
            pinOrDie(cfg.consumerCpu);
            consumer(*ring, sh);
        });
        sh.gate.open(2);
        p.join();
        c.join();
        return;
    }

    // Processes: this process owns the segment, each child opens it by name like a real stage would
    std::string err;
    auto owner = ShmRing<Ring>::create(kRingName, cfg.capacity, &err, true);
    if (!owner) die(err);
    auto side = [&](const char* name, int cpu, auto&& body) {
        std::fflush(stdout);
        pid_t pid = fork();
        if (pid != 0) return pid;
        setThreadName(name);
        pinOrDie(cpu);
        std::string e;
        auto mine = ShmRing<Ring>::open(kRingName, &e);
        if (!mine) die(std::string(name) + ": " + e);
        body(mine->ring(), sh);
        std::fflush(stdout);
        _exit(0);  // skip the parent's destructors (it owns the segment, not us)
    };
    pid_t p = side("producer", cfg.producerCpu, producer);
    pid_t c = side("consumer", cfg.consumerCpu, consumer);
    sh.gate.open(2);
    for (pid_t pid : {p, c}) {
        int status = 0;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) die("a benchmark process failed");
    }
}

// ---------------------------------------------------------------- throughput

static double throughputRun(const Config& cfg, uint64_t count) {
    SharedBlock block;
    Shared& sh = *block;
    runPair(
        cfg, sh,
        [count](Ring& ring, Shared& s) {
            Msg m{};
            m.hdr.type = MsgType::NewOrder;
            s.gate.arriveAndWait();
            s.start = now();
            for (uint64_t seq = 0; seq < count; ++seq) {
                m.hdr.seq = seq;
                while (!ring.tryPush(m)) {
                }
            }
        },
        [count](Ring& ring, Shared& s) {
            Msg m;
            s.gate.arriveAndWait();
            for (uint64_t got = 0; got < count;)
                if (ring.tryPop(m)) ++got;
            s.end = now();
        });
    return double(count) / ticksToSec(sh.end - sh.start);
}

// ---------------------------------------------------------------- latency

struct LatencyResult {
    double targetRate = 0;
    double achievedRate = 0;
    double maxLagNs = 0;  // how far the producer fell behind its schedule
    Percentiles scheduled, actual;
};

static LatencyResult latencyRun(const Config& cfg, double rate, uint64_t count, double warmupPct) {
    SharedBlock block;
    Shared& sh = *block;
    double intervalTicks = ticksPerNs() * 1e9 / rate;
    runPair(
        cfg, sh,
        [=](Ring& ring, Shared& s) {
            Msg m{};
            m.hdr.type = MsgType::NewOrder;
            uint64_t maxLag = 0;
            s.gate.arriveAndWait();
            uint64_t start = now();
            s.start = start;
            for (uint64_t i = 0; i < count; ++i) {
                uint64_t due = start + uint64_t(double(i) * intervalTicks);
                uint64_t t;
                while ((t = now()) < due) {  // pace: spin until this message is due
                }
                if (t - due > maxLag) maxLag = t - due;
                m.hdr.seq = i;
                m.hdr.tsFeeder = due;          // scheduled send time
                m.newOrder.clientOrderId = t;  // actual send time (benchmark-only use of the field)
                while (!ring.tryPush(m)) {
                }
            }
            s.maxLag = maxLag;
        },
        [=](Ring& ring, Shared& s) {
            Samples fromScheduled(count), fromActual(count);  // allocated (and touched) before the start
            Msg m;
            s.gate.arriveAndWait();
            for (uint64_t got = 0; got < count;) {
                if (!ring.tryPop(m)) continue;
                uint64_t t = now();
                fromScheduled.add(t - m.hdr.tsFeeder);
                fromActual.add(t - m.newOrder.clientOrderId);
                ++got;
            }
            s.end = now();
            s.scheduled = fromScheduled.percentiles(warmupPct);
            s.actual = fromActual.percentiles(warmupPct);
        });

    LatencyResult r;
    r.targetRate = rate;
    r.achievedRate = double(count) / ticksToSec(sh.end - sh.start);
    r.maxLagNs = ticksToNs(sh.maxLag);
    r.scheduled = sh.scheduled;
    r.actual = sh.actual;
    return r;
}

// ---------------------------------------------------------------- reporting

static std::vector<double> parseList(const std::string& s) {
    std::vector<double> out;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t comma = s.find(',', pos);
        std::string item = s.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        char* endp = nullptr;
        double v = std::strtod(item.c_str(), &endp);
        if (item.empty() || *endp != '\0' || v <= 0) {
            std::fprintf(stderr, "bad --loads item '%s'\n", item.c_str());
            std::exit(2);
        }
        out.push_back(v);
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return out;
}

static std::string cpuLabel(int cpu) { return cpu < 0 ? "any" : std::to_string(cpu); }

class Csv {
public:
    Csv(const std::string& path, const Config& cfg) : cfg_(cfg) {
        if (path.empty()) return;
        f_ = std::fopen(path.c_str(), "a");
        if (!f_) die("cannot open " + path);
        if (std::ftell(f_) == 0)
            std::fprintf(f_,
                         "test,build,mode,capacity,producer_cpu,consumer_cpu,target_rate,msgs_per_s,timed_from,"
                         "samples,mean_ns,p50_ns,p90_ns,p99_ns,p999_ns,p9999_ns,max_ns\n");
    }
    ~Csv() {
        if (f_) std::fclose(f_);
    }

    void throughput(double msgsPerSec) {
        if (f_) std::fprintf(f_, "%s,0,%.0f,,,,,,,,,\n", prefix("throughput").c_str(), msgsPerSec);
    }
    void latency(const LatencyResult& r, const char* from, const Percentiles& p) {
        if (!f_) return;
        std::fprintf(f_, "%s,%.0f,%.0f,%s,%llu,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f\n", prefix("latency").c_str(),
                     r.targetRate, r.achievedRate, from, (unsigned long long)p.count, p.mean, p.p50, p.p90,
                     p.p99, p.p999, p.p9999, p.max);
    }

private:
    std::string prefix(const char* test) const {
        return std::string(test) + "," + LAB_BUILD + "," + modeName(cfg_.mode) + "," +
               std::to_string(cfg_.capacity) + "," + cpuLabel(cfg_.producerCpu) + "," + cpuLabel(cfg_.consumerCpu);
    }

    Config cfg_;
    FILE* f_ = nullptr;
};

static void printEnvironment(const Config& cfg) {
    // Cost of reading the clock: two back-to-back reads
    Samples overhead(1'000'000);
    for (int i = 0; i < 1'000'000; ++i) {
        uint64_t a = now();
        uint64_t b = now();
        overhead.add(b - a);
    }
    Percentiles o = overhead.percentiles(10);
    std::string gov = cpuGovernor();
    const char* where = cfg.mode == Mode::Heap        ? "heap memory, two threads"
                        : cfg.mode == Mode::Shm       ? "shared memory, two threads"
                                                      : "shared memory, two processes";

    std::printf("cpu        %s, %d CPUs online\n", cpuModel().c_str(), onlineCpus());
    if (!gov.empty()) std::printf("governor   %s\n", gov.c_str());
    std::printf("clock      %s\n", clockDescription().c_str());
    std::printf("clock cost p50 %.1f ns, p99 %.1f ns (two back-to-back reads)\n", o.p50, o.p99);
    std::printf("build      %s\n", LAB_BUILD);
    std::printf("ring       MutexRing<Msg>, capacity %u, %s\n", cfg.capacity, where);
    std::printf("pinning    producer CPU %s, consumer CPU %s\n\n", cpuLabel(cfg.producerCpu).c_str(),
                cpuLabel(cfg.consumerCpu).c_str());
}

static void printLatencyRow(const char* load, const LatencyResult& r, const char* from, const Percentiles& p) {
    std::printf("  %6s %11.0f %11.0f  %-9s %8.0f %8.0f %8.0f %9.0f %10.0f %10.0f %8.0f\n", load, r.targetRate,
                r.achievedRate, from, p.p50, p.p90, p.p99, p.p999, p.p9999, p.max, p.mean);
}

int main(int argc, char** argv) {
    Args args(argc, argv, "Ring buffer benchmark: throughput, then latency at several loads");
    Config cfg;
    cfg.capacity = args.get<uint32_t>("capacity", 1024, "ring capacity in messages");
    cfg.producerCpu = args.get<int>("producer-cpu", -1, "pin the producer (-1 = don't pin)");
    cfg.consumerCpu = args.get<int>("consumer-cpu", -1, "pin the consumer (-1 = don't pin)");
    bool shm = args.flag("shm", "ring in shared memory (still two threads)");
    bool processes = args.flag("processes", "producer and consumer as two processes (ring in shared memory)");
    uint64_t count = args.get<uint64_t>("count", 2'000'000, "messages per throughput run");
    int repeats = args.get<int>("repeats", 5, "throughput runs (after one warm-up run)");
    std::string loads = args.get<std::string>("loads", "10,50,80,95", "latency loads, % of median throughput");
    double rate = args.get<double>("rate", 0, "latency at this one rate in msgs/s (skips throughput)");
    uint64_t latencyMsgs = args.get<uint64_t>("latency-msgs", 500'000, "messages per latency run");
    double warmup = args.get<double>("warmup", 10, "% of latency samples discarded as warm-up");
    bool throughputOnly = args.flag("throughput-only", "skip the latency runs");
    std::string csvPath = args.get<std::string>("csv", "", "append results to this CSV file");
    args.done();
    cfg.mode = processes ? Mode::Processes : shm ? Mode::Shm : Mode::Heap;
    if (cfg.capacity == 0 || repeats < 1 || count == 0 || latencyMsgs == 0 || warmup < 0 || warmup >= 100) {
        std::fprintf(stderr, "invalid option values\n");
        return 2;
    }
    for (int cpu : {cfg.producerCpu, cfg.consumerCpu})
        if (cpu >= 0 && (!pinningSupported() || cpu >= onlineCpus()))
            die("cannot pin to CPU " + std::to_string(cpu) + " on this machine");
    std::vector<double> loadPcts = parseList(loads);

    calibrate();
    printEnvironment(cfg);
    Csv csv(csvPath, cfg);

    double maxRate = 0;
    if (rate <= 0) {
        std::printf("throughput: %llu messages per run, 1 warm-up + %d runs\n", (unsigned long long)count, repeats);
        throughputRun(cfg, count / 10 + 1);  // warm-up: page faults, caches, CPU clock ramp
        std::vector<double> runs;
        for (int i = 0; i < repeats; ++i) {
            runs.push_back(throughputRun(cfg, count));
            csv.throughput(runs.back());
            std::printf("  run %d  %12.0f msgs/s\n", i + 1, runs.back());
        }
        std::sort(runs.begin(), runs.end());
        maxRate = runs[runs.size() / 2];
        std::printf("  median %10.0f msgs/s  (min %.0f, max %.0f, spread %.1f%%)\n\n", maxRate, runs.front(),
                    runs.back(), 100.0 * (runs.back() - runs.front()) / maxRate);
    }
    if (throughputOnly) return 0;

    std::printf("latency: %llu messages per load, first %.0f%% discarded, ns\n", (unsigned long long)latencyMsgs,
                warmup);
    std::printf("  %6s %11s %11s  %-9s %8s %8s %8s %9s %10s %10s %8s\n", "load", "target/s", "achieved/s",
                "timed", "p50", "p90", "p99", "p99.9", "p99.99", "max", "mean");

    auto runOne = [&](const char* label, double r) {
        LatencyResult res = latencyRun(cfg, r, latencyMsgs, warmup);
        printLatencyRow(label, res, "scheduled", res.scheduled);
        printLatencyRow("", res, "actual", res.actual);
        if (res.maxLagNs > 1e6) std::printf("  %6s producer fell %.1f ms behind schedule\n", "", res.maxLagNs / 1e6);
        csv.latency(res, "scheduled", res.scheduled);
        csv.latency(res, "actual", res.actual);
    };
    if (rate > 0) {
        runOne("fixed", rate);
    } else {
        for (double pct : loadPcts) {
            char label[16];
            std::snprintf(label, sizeof label, "%.0f%%", pct);
            runOne(label, maxRate * pct / 100.0);
        }
    }
    return 0;
}
