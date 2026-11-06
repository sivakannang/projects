# Task 3: Common helpers

| File | What it does |
|---|---|
| `common/args.hpp` | Tiny `--key=value` / `--flag` parser with `--help` |
| `common/log.hpp` | `LOG_INFO` / `LOG_WARN` / `LOG_ERROR` to stderr |
| `common/cpu.hpp/.cpp` | Pin a thread to a CPU, read back where it runs, name threads |
| `common/process.hpp` | `startProcess(name, cpu)`: the startup every program does |

Every program now starts like this:
```cpp
lab::Args args(argc, argv, "description");
int cpu = args.get<int>("cpu", -1, "CPU to pin this process to (-1 = don't pin)");
args.done();
lab::startProcess("processor", cpu);
```

## Option parsing
- Options are declared where they are read, then `done()` rejects anything undeclared.
- A typo (`--rat=5`), a bad value (`--cpu=abc`) or a missing value (`--cpu`) exits with code 2
  and a clear message. **Why it matters:** a silently ignored option means you benchmark a
  different setup than you think, and nothing tells you.

## Logging
- Format: `21:22:26.349 INFO  [processor 10243] message`
- One `write()` per line, so lines from different processes never mix mid-line.
- **Never on the hot path.** One log call costs microseconds: clock read, `printf`-style
  formatting, and a `write()` system call into the kernel. That is the entire per-order budget.
  Per-order information goes into shared-memory counters (task 14). Real systems use an
  asynchronous binary logger (task 38).

## CPU pinning
- Linux: `pthread_setaffinity_np` gives a hard pin. We read it back with
  `pthread_getaffinity_np` ("allowed CPUs") and `sched_getcpu` ("running on CPU") to prove it.
- macOS: no pinning API exists, so `--cpu` only prints a warning.
- A failed pin on Linux is fatal: a benchmark running on the wrong CPU is worse than no benchmark.
- Pinned in code, not with `taskset`, because `taskset` pins a whole process, while the feeder
  will need its two threads (send / receive) on different CPUs.
- Threads are named (`pthread_setname_np`) so they show by name in `htop`, `perf` and `gdb`.

## Why pinning matters for latency
Without pinning, the scheduler can move a thread to another CPU at any time. After a move:
- its L1/L2 caches on the new CPU are cold, so the next accesses miss;
- it may land on a hyper-thread sibling of another busy thread and run slower;
- `perf` shows these as `cpu-migrations`.
Pinning removes all three, which mostly shows up as a lower p99/p99.9.

## Verified on the Dell (i3-4005U)
```
--cpu=0..3  -> allowed CPUs N, running on CPU N
--cpu=4     -> ERROR cannot pin to CPU 4: Invalid argument (this machine has 4 CPUs: 0-3)
no --cpu    -> allowed CPUs 0-3
```
