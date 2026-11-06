# Task 7: Timing helpers and the first ring benchmark

Files: `common/timing.hpp`, `common/stats.hpp`, `bench/ring_bench.cpp`, `make bench`,
`cpuModel()` / `cpuGovernor()` in `common/cpu.*`.

## Clock (`timing.hpp`)
- `now()` = `rdtsc` on x86, `steady_clock` in ns elsewhere. Returns **ticks**.
- `calibrate()` once at startup: ticks counted over 200 ms of `steady_clock` → ticks per ns.
- `invariantTsc()` reads CPUID; the Dell has `constant_tsc` + `nonstop_tsc`, so ticks taken on
  different cores (later: different processes) can be subtracted.

| Machine | Clock | Ticks/ns | Two back-to-back reads |
|---|---|---|---|
| Dell i3-4005U | rdtsc | 1.696 (= 1.7 GHz base clock) | 14 ns, so ~7 ns per read |
| Mac M4 | steady_clock | 1 | 41 ns (24 MHz counter: one step is ~42 ns) |

## Statistics (`stats.hpp`)
`Samples` pre-allocates the buffer; `add()` is one store. `percentiles(warmupPct)` drops the first
part of the run, sorts, and reads p50 / p90 / p99 / p99.9 / p99.99 / max / mean in ns.

## Benchmark (`bench/ring_bench.cpp`)
1. Prints the environment: CPU, governor, clock, clock cost, build, ring, pinning.
2. **Throughput:** 1 warm-up run, then `--repeats` runs of `--count` messages. Prints the median and the spread.
3. **Latency:** paced sending at `--loads` percent of the median throughput (or one fixed `--rate`).
   Each message carries two stamps:
   - **scheduled**: when it was due. This is correct and includes coordinated omission.
   - **actual**: when the producer really sent it. This is the naive number.
4. `--csv=file` appends every result, so runs can be compared later.

```
build/release/bin/bench/ring_bench --producer-cpu=0 --consumer-cpu=1 --csv=results.csv
```

## Dell results (MutexRing, capacity 1024, governor ondemand)

### Throughput (2M messages, 5 runs)
| Placement | Median msgs/s | Spread (max−min)/median |
|---|---|---|
| unpinned | 2.88M | **72%** (1.59M … 3.65M) |
| different cores (0 / 1) | **4.04M** | **2.9%** |
| hyper-threads of one core (0 / 2) | 0.48M | 36% |

Throughput is higher than `ring_test` (1.87M) because the benchmark doesn't fill or verify each message.

### Latency vs load, different cores (0 / 1), ns
Loads are % of the measured 3.67M msgs/s.

| Load | Rate | p50 | p90 | p99 | p99.9 | max |
|---|---|---|---|---|---|---|
| 5% | 184K | 2,093 | 3,104 | 6,478 | 18,952 | 293,661 |
| 10% | 367K | 2,059 | 3,383 | 8,144 | 19,278 | 45,088 |
| 20% | 735K | 3,276 | 7,915 | 15,449 | 26,324 | 55,953 |
| 30% | 1.10M | 5,413 | 14,330 | 27,357 | 39,788 | 52,808 |
| 40% | 1.47M | 8,849 | 24,045 | 70,347 | 210,120 | 253,565 |
| 48% | 1.76M | 15,338 | 32,138 | 59,968 | 79,991 | 90,401 |
| 50%+ | 2.0M+ | **7–60 ms**, ring full, producer falls behind | | | | |

All figures use the scheduled stamp.

## Lessons
1. **Pinning buys repeatability more than speed.** Unpinned runs swing between 1.6M and 3.7M
   (72% spread); pinned runs stay within 3%. You can't measure a 5% optimisation with a 72% noise band.
2. **The "max throughput" from a blast test is not the capacity.**
   - The blast test says 4M msgs/s. With pacing and timestamps the same ring only sustains about 2M/s.
     Each side does extra work per message (reading the clock, pacing, storing samples), and the lock is contended in a different pattern.
   - Capacity depends on the exact work done per message. That's why a real capacity figure is
     "the highest rate at which p99 stays acceptable", measured with the real workload.
     NSE's 270K/330K is that kind of number.
3. **Latency rises long before the maximum.** p50 stays around 2 µs up to 10% load, then roughly doubles by 30% and is 15 µs at 48%.
   Queues start to form well before saturation (queueing theory: wait grows as 1/(1 − utilisation)).
4. **Past capacity, latency equals the full queue.**
   - With the ring full, the naive "actual" latency settles at about 500 µs. That's 1024 slots ÷ ~2M msgs/s: **Little's law**, wait = queue length ÷ service rate.
   - A bigger ring wouldn't make anything faster, only longer to wait in.
5. **Coordinated omission is huge.** At 80% load (unpinned) the naive p99 is 0.58 ms, but the true (scheduled) p99 is 26.9 ms.
   - The naive number stays flat because the producer stamps messages late.
   - The scheduled number keeps growing for as long as the overload lasts, which is what real clients would experience.
6. **Hyper-threads hurt even at low load.** At 10% load the scheduled p99 is 70 µs vs 6 µs actual:
   the spinning consumer on the sibling hyper-thread delays the producer, so it misses its schedule.
7. **Small effects add up:** each clock read is ~7 ns on the Dell, and every message pays for three of them.
   On the Mac a read costs ~41 ns, which matters when a message only takes ~25 ns.

## Mac (M4, unpinned, informational)
Throughput 42M msgs/s median. Paced latency is dominated by the 41 ns clock reads.
It shows the same coordinated-omission gap: at 50% load, actual p99 is 71 µs and scheduled p99 is 8.3 ms.
