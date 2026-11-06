# Task 6: Baseline ring buffer (mutex)

Files: `common/ring_mutex.hpp`, `tests/ring_test.cpp`, `make BUILD=tsan`.

## Design
```
[ header: mutex | capacity | head | tail | count ][ slot 0 ][ slot 1 ] ... [ slot N-1 ]
```
- One contiguous block, no pointers → can be placed in shared memory as-is (task 9).
- `pthread_mutex_t` with `PTHREAD_PROCESS_SHARED`: works between threads *and* processes.
  `std::mutex` can't be told that.
- `tryPush` / `tryPop` never wait: they return `false` when full / empty and the caller decides
  how to wait. Baseline caller just retries (busy-spin). Other waiting strategies: task 28.
- Every push/pop takes the lock, so producer and consumer serialize on one mutex.
- `% capacity` for wrap-around (the `& (capacity-1)` mask is a later optimisation).
- Slots copied with `memcpy` (the message is trivially copyable).

## Test
Producer thread pushes `seq = 0..N-1` with content derived from `seq`; consumer checks each message
is exactly the next `seq` with identical bytes → no loss, duplicates, reordering or corruption.
Capacities 1, 2, 3 (constantly full/empty, wrapping every message) and 1024.

## Dell results (i3-4005U, release, 2M messages, msgs/s)
| Placement | cap 1 | cap 2 | cap 3 | cap 1024 |
|---|---|---|---|---|
| unpinned | 381K | 512K | 530K | 1.83M |
| different cores (CPU 0 / CPU 1) | 356K | 463K | 537K | **1.87M** |
| same core, hyper-threads (CPU 0 / CPU 2) | 381K | 416K | 428K | **0.56M** |
| same CPU (CPU 1 / CPU 1), 2K messages | **194** | 386 | 579 | 283K |

Debug and TSan builds: all correct, ThreadSanitizer reports no data races
(header is 56 bytes on Linux, 80 on macOS: `pthread_mutex_t` differs in size).

## Lessons from the pinning runs
1. **Unpinned ≈ different cores** on an idle machine: the scheduler already spreads two busy
   threads onto separate cores. Pinning pays off under load and noise (the p99 tail), not in the
   average on a quiet machine. Task 13 will show that.
2. **Hyper-threads of one core are 3.3× slower at capacity 1024.** Passing data is cheap (shared
   L1), but both threads share one core's execution units, and the spinning consumer
   (11.9M empty spins) steals cycles from the producer doing real work. This is why low-latency
   setups disable hyper-threading or leave the critical thread's sibling idle.
3. **Two spinning threads on one CPU: 194 msgs/s, about 5 ms per message.** A busy-spinning
   thread never gives up the CPU, so the other one only runs when the scheduler's time slice
   expires (milliseconds). Rule: never have more spinning threads than CPUs. Yielding or
   blocking (task 28) avoids this, at the cost of latency.
4. Mac vs Dell at capacity 1024: 24.4M vs 1.9M msgs/s. A 2014 laptop core vs a modern one,
   plus different mutex implementations.

## Mac results (M-series, unpinned)
| Build | cap 1 | cap 2 | cap 3 | cap 1024 |
|---|---|---|---|---|
| release | 126K msgs/s | 259K | 395K | 24.4M |
| debug (ASan) | 936K | 969K | 909K | 3.9M |
| tsan | 106K | 172K | 278K | 596K |

All correct; ThreadSanitizer reports no data races.

## Lesson: release slower than debug at capacity 1
With one slot the producer and consumer must strictly alternate. Both threads spin on
`tryPush`/`tryPop` as fast as they can, so they fight for the mutex all the time
(860M failed pushes for 2M messages). The faster the code (release), the harder they hammer the
lock: the cache line holding the mutex bounces between cores and the lock gets handed back and
forth (macOS mutexes are fair, so a waiter may be parked and woken by the kernel).
The slower debug build accidentally backs off and contends less.

Take-aways:
- Contention, not the work itself, dominates when two threads share one lock.
- "Faster code" can be slower under contention. Always measure.
- A bigger ring lets the two sides work on different slots most of the time: 24M msgs/s at 1024.
- Lock-free rings (task 27) remove the shared lock entirely.
