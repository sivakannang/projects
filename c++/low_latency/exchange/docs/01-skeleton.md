# Task 1: Skeleton

## What we did
Created the folder layout and one stub program per process.

| Folder | Holds |
|---|---|
| `apps/` | one `.cpp` per process: feeder, receiver, processor, sender, monitor |
| `common/` | code shared by all processes: messages, ring buffer, shared memory, timing, stats |
| `engine/` | the matching engine (order pool, order book, validator) |
| `tests/` | unit tests |
| `scripts/` | run / capacity / perf scripts |
| `docs/` | one note per task |

## Why separate processes (not threads)
- Mirrors the NSE setup: each stage is its own process, linked by shared-memory ring buffers.
- A crash in one stage doesn't corrupt another's private memory.
- Each process can be pinned, restarted and profiled on its own (`perf stat -p <pid>`).
- Cost: anything shared must live in explicitly mapped shared memory, and pointers
  can't be shared (each process may map the region at a different address), so shared
  structures use indices/offsets instead of pointers.
