# Task 9: Ring buffer in shared memory, two processes

Files: `common/shm_ring.hpp`, `MutexRing::attach()` (+ `slotSize_`) in `common/ring_mutex.hpp`,
`tests/shm_ring_test.cpp`, `tests/ring_check.hpp` (shared message checks),
`ring_bench --shm` / `--processes`.

## Layout
```
/dev/shm/lab_feed
[ ShmHeader 64 B ][ MutexRing header: mutex | capacity | slotSize | head | tail | count ][ slot 0 ] ...
```
- **Owner:** `ShmRing<MutexRing<Msg>>::create(name, capacity)` creates the segment, builds the ring in place and marks it ready.
  Its destructor unlinks the name.
- **Other side:** `ShmRing<...>::open(name)` opens the segment, then `MutexRing::attach()` uses the ring as it is.
  Calling `create` again would reset the mutex and indices while the owner is using them.
- `attach()` checks the slot size and the capacity. A build with a different `Msg` size is refused:
  "ring slots are 8 bytes, this build expects 64".
- **Robust mutex:** not done, by choice. If a process dies holding the lock, the other side waits for ever.
  Linux `PTHREAD_MUTEX_ROBUST` could recover, but a crashed stage means restarting the pipeline, and the lock-free rings (task 27) have no lock.

## Correctness
`shm_ring_test`: the producer is this process, the consumer is a `fork()`ed process that opens the ring by name.
- Every seq arrives in order with verified content, at capacities 1, 2, 3 and 1024.
- If the consumer dies, the producer notices instead of spinning on a full ring for ever.
- Passes on the Mac and the Dell (release and debug), and leaves `/dev/shm` clean.

## Dell benchmark: threads vs processes (pinned CPU 0 / CPU 1, governor ondemand)
`ring_bench` modes change one thing at a time:

| Mode | Throughput median | p50 / p99 at 10% load |
|---|---|---|
| default: threads, heap ring | 3.86M msgs/s | 2.0 / 8.1 µs |
| `--shm`: threads, shared memory ring | 3.52M (spread 12.7%) | 2.0 / 7.1 µs |
| `--processes`: two processes, shared memory ring | 3.97M | 2.1 / 8.9 µs |

At the same absolute rate (scheduled stamps, µs):

| Rate | Mode | p50 | p99 |
|---|---|---|---|
| 1.15M/s (3 runs each) | threads-shm | 5.4–6.6 | 27–32 |
| | processes | 6.3–8.2 | 30–45 |
| 1.7M/s (near the knee) | heap / shm / processes | 10–12 / 11–12 / 12–24 | 42–46 / 46–52 / 52–109 |

Context switches during a 1.7M/s run (`/usr/bin/time -v`): about 50 voluntary and 300 involuntary in every mode, so nobody sleeps on the mutex much.

## Lessons
1. **Threads vs processes: the same speed.** Throughput and low-load latency are equal within noise.
   - Once the memory is shared, the CPU only sees two cores passing a cache line back and forth.
   - Whether those cores run threads of one process or two processes doesn't matter.
   - Separate processes buy isolation, independent restarts and deployment; they don't cost speed.
2. **Near saturation, single runs lie.**
   - The first `--processes` run looked 10× worse at 30% load. It was really run-to-run noise.
   - The same rate repeated three times overlapped with threads.
   - The 45% load runs used different absolute rates, because each mode's throughput median differed.
   - **Compare modes at the same absolute rate, and repeat.**
3. Processes may be a little worse right at the knee (1.7M/s). The evidence is weak (2–3 runs). It's worth revisiting with `perf` once it's enabled.
4. **`perf` is blocked on Ubuntu by default** (`perf_event_paranoid = 4`). Enabling it is a one-line sudo command (below).

## Mac note
- Across processes, capacity 1 ran at 3.7M msgs/s, vs 126K between threads in task 6.
- The macOS process-shared mutex probably behaves differently from its private one under contention. That's not investigated, since the Dell gives the reference numbers.

## Commands
```
make && make test
build/release/bin/bench/ring_bench --producer-cpu=0 --consumer-cpu=1
build/release/bin/bench/ring_bench --producer-cpu=0 --consumer-cpu=1 --shm
build/release/bin/bench/ring_bench --producer-cpu=0 --consumer-cpu=1 --processes
build/release/bin/bench/ring_bench --processes --producer-cpu=0 --consumer-cpu=1 --rate=1150000
```
