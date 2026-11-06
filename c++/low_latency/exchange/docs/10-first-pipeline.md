# Task 10: First pipeline (pass-through)

Files: `common/pipeline.hpp`, `apps/{feeder,receiver,processor,sender}.cpp`, `tests/pipeline_test.sh`,
`MsgType::EndOfStream`, `SharedMemory::open()` now waits for the name and skips dead creators.

```
  feeder ──/lab_f2r──▶ receiver ──/lab_r2p──▶ processor ──/lab_p2s──▶ sender
    ▲                                                                   │
    └─────────────────────────────/lab_s2f──────────────────────────────┘
```

## Decisions
| Topic | Choice | Why |
|---|---|---|
| Ring ownership | Each program **creates the ring it reads** and opens the one it writes (waiting up to `--wait-ms`) | Start order doesn't matter; each program unlinks its own ring on exit |
| Stale rings after a crash | Creator uses `replace`. `open()` treats a ring whose creator pid is dead as "not there yet" | A writer can never attach to an orphaned ring that nobody reads |
| Feeder threads | One thread: send if there's room, otherwise collect a reply | Dell has 4 logical CPUs, so 4 programs = 4 spinners. More spinners than CPUs collapses throughput (task 6) |
| Shutdown | `EndOfStream` message travels through the rings; Ctrl-C/SIGTERM sets a flag | In-band, so it can't overtake orders; nothing in flight is lost. Ctrl-C still cleans up |
| Processor | Replies with a dummy `ExecReport` (Accepted / Modified / Cancelled), in place | Return path already has the real format; the matching engine slots in at task 25 |

Each stage stamps `tsReceiver` / `tsProcessor` / `tsSender` (ticks since `tsFeeder`). They are analysed in task 13.

## Checks
- **Feeder:** sent = received = count, replies come back as seq 0, 1, 2, … in order, the `clientOrderId` matches the order sent with that seq, and the status matches the request type.
- **Every stage:** counts seq gaps. At exit it logs messages handled, gaps, input-empty spins and output-full spins.
- **`tests/pipeline_test.sh`** (part of `make test`): starts all four, sends 200K orders, checks every exit code and that `/dev/shm` has no `lab_*` left. It has a 60 s watchdog so it never hangs.
- **Also tried by hand:** feeder started first; Ctrl-C mid-run (every program logs "stopped" and cleans up); a stage whose output ring never appears (times out with a clear error).

## Dell results (2M orders, release, governor ondemand, round trip msgs/s, 3 runs)
| Layout | Runs |
|---|---|
| unpinned | 0.97M, 1.01M, 1.11M |
| feeder 0, receiver 1, processor 2, sender 3 | 0.97M, 1.03M, 1.06M |
| feeder 0, receiver 2, processor 1, sender 3 | 1.13M, 1.03M, 0.91M |

Mac M4: about 5M msgs/s round trip.

## Lessons
1. **The pipeline is only as fast as its slowest hop, and on the Dell every hop is crowded.**
   - One ring between two cores did 3.9M msgs/s (task 9). Here there are 4 rings and 4 spinning programs on 2 physical cores, so every core runs two of them as hyper-threads.
   - Task 6 measured hyper-thread pairs at ~3.3× slower: 3.9M / 3.3 ≈ 1.2M, close to the ~1.0M seen.
2. **Pinning layouts make no difference here.** Every layout puts two spinners on each core, so they're all equally cramped.
   The HP (2 P-cores + 8 E-cores) can give every program its own core; that's the real pinning experiment (task 29).
3. Empty and full spin counts show where the pressure is.
   - **Mac run:** the receiver and processor mostly found their input **empty**, so they were waiting for work.
   - **Ctrl-C run:** the sender found its output **full** 22M times, so it was blocked by the feeder, which can only collect one reply per loop.
   - Task 14 turns these into live per-stage counters.

## Run it
```bash
cd ~/lab/c++/low_latency/exchange && make
build/release/bin/receiver & build/release/bin/processor & build/release/bin/sender & build/release/bin/feeder --count=2000000
```
Pinned (one program per logical CPU):
```bash
build/release/bin/receiver --cpu=1 & build/release/bin/processor --cpu=2 & build/release/bin/sender --cpu=3 & build/release/bin/feeder --cpu=0 --count=2000000
```
