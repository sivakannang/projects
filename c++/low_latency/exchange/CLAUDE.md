# Exchange lab

A learning lab for low-latency systems: a small exchange pipeline over shared-memory SPSC ring
buffers, used to learn benchmarking, latency, throughput, profiling and optimisation step by step.
The owner helped raise an NSE F&O matching engine from 270K to 330K orders/s and wants to be able
to explain that kind of work in interviews from first principles.

```
feeder ──/lab_f2r──▶ receiver ──/lab_r2p──▶ processor ──/lab_p2s──▶ sender
  ▲                                                                   │
  └─────────────────────────────/lab_s2f──────────────────────────────┘
```

## Where things are
- `TASKS.md`: the plan (38 small tasks). Ticked tasks are done.
- `docs/NN-*.md`: one note per task: design decisions, measured results, lessons. Read the latest few to catch up.
- `apps/`: feeder, receiver, processor, sender, monitor (one process each).
- `common/`: shared helpers (options, logging, pinning, timing, stats, shm, rings, pipeline loop).
- `engine/`: the matching engine (from task 18 on). It is new, not a copy of `c++/trading`.
- `bench/`, `tests/`, `scripts/run.sh` (start, watch and stop one experiment).

## Commands
```bash
make                      # release build -> build/release/bin
make test                 # unit tests + pipeline test
make BUILD=debug test     # ASan + UBSan
make BUILD=tsan test      # ThreadSanitizer
scripts/run.sh --cpus=0,1,2,3 -- --rate=500000 --duration=10 --progress
scripts/run.sh --clean    # kill stray lab programs, remove leftover rings
```
Every run goes into `runs/<date_time>/` (git-ignored): command, environment and one log per program.

## How we work
- **One task at a time, discussed before coding.** Explain the options and trade-offs, recommend one, and build after agreement.
- **Measure, don't guess.** Every claim gets a number with its environment: machine, pinning, governor, build.
- **Change one thing at a time**, with before/after numbers.
- **After each task:**
  - write `docs/NN-*.md`: decisions, results table, lessons, how to run;
  - tick it in `TASKS.md`.
- **Commit only when asked.**
- **Explain in plain English,** with the "why" and the interview angle.

## Machines
- **Mac M4 (development):** there is no CPU pinning on macOS, and the clock is steady_clock (~41 ns per read).
- **Dell i3-4005U (Ubuntu):** 2 cores / 4 threads. CPU 0 and 2 are core 0; CPU 1 and 3 are core 1.
  - Four spinning programs fill every CPU, so results are capped at ~1.0M msgs/s round trip, with OS-noise stalls.
- **HP i5-1235U (from Nov 2026):** 2 P-cores + 8 E-cores.
- **Office UAT hosts (32–64 cores):** the first chance to give each program its own physical core.
  - Pick CPUs on one NUMA node.
  - Check the cores aren't used by anything else; `run.sh` warns.
  - Ask the host owners before running: each run keeps 4 cores at 100%.
