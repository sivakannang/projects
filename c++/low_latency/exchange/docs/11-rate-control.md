# Task 11: Feeder rate control

Files: `apps/feeder.cpp` (rewritten), `MsgType::Ping`, `Rng::unit()`.

## Options
```
feeder --rate=500000 --duration=10              # 500K orders/s for 10 s, constant gaps
feeder --rate=500000 --duration=10 --arrival=poisson
feeder --rate=500000 --duration=10 --progress   # one line per second
feeder --duration=5                             # as fast as possible for 5 s
feeder --count=2000000                          # as fast as possible, 2M orders (as in task 10)
feeder --pool=1000000                           # orders generated up front, replayed in a loop
```

## Design
- **Pacing:** order i is due at a scheduled time.
  - **Constant:** `start + i / rate`.
  - **Poisson:** random exponential gaps with the same average rate. They come from their own seeded Rng, so a run is repeatable.
  - The feeder busy-waits on `rdtsc` and collects replies in the gaps.
- **Stamp = scheduled time**, not the actual send time. If the pipeline stalls, the feeder **catches up**
  (sends overdue orders as soon as there is room) and the stall shows up in latency: no coordinated omission.
- **Replay:** a pool of `--pool` orders (64 MB per 1M) is generated once and replayed in a loop. Each pass shifts every `clientOrderId` by the pool's highest id, so ids stay unique and replies can still be checked.
- **Start barrier:** one `Ping` goes round the whole loop before the clock starts.
  - Without it, a paced run began while later stages were still connecting, and the feeder was 7–8 ms behind in its first second.
- **Report:** target vs achieved rate, and the largest lag behind schedule.
  - Achieved < 99% of target means **"could not keep up"**: the pipeline is saturated.
  - A lag over 1 ms with the average rate met means **a stall happened**, then it caught up.
- **`--progress`:** one line per second. Each line costs a few µs of send-loop time, which is why it's opt-in. The real live view is the monitor (tasks 14–15).

## Dell results (pinned feeder 0, receiver 1, processor 2, sender 3; 5 s each)
| Target | Achieved | Max behind schedule | Verdict |
|---|---|---|---|
| 50K/s | 50K | 17.5 ms | kept up, stalled once |
| 200K/s | 200K | 2.9 ms | kept up |
| 500K/s | 500K | 100 ms | kept up, stalled once |
| 800K/s | 800K | 212 ms | kept up; one second dropped to 670K, then 930K to catch up |
| 1.0M/s | 804K | 1.2 s | **could not keep up** |
| 1.2M/s | 910K | 1.6 s | **could not keep up** |
| 500K/s Poisson | 500K | 5.8 ms | kept up |

All runs: every reply in order, 0 errors. Unpaced capacity (task 10) was ~1.0M round trip, so the Dell
saturates between 0.8M and 1.0M orders/s.

## Lessons
1. **Capacity has a sharp edge.** Up to ~800K/s the pipeline keeps up on average. At 1.0M it falls further behind every second (1.2 s by the end). Task 16 automates finding that edge.
2. **Stalls happen even at 50K/s, a 5% load.**
   - All 4 CPUs run a spinning stage, so when Linux runs anything else (GNOME, kworkers, sshd, timer ticks), it **preempts a stage** for milliseconds.
   - The whole loop waits behind it: one second showed 1,158 messages in flight and 161 ms behind.
   - This is OS noise, not our code. Fixes come in task 35: isolated cores, no desktop, `nohz_full`, and a machine with spare cores.
   - Interview line: *"the average rate was fine, the tail was the OS"*.
3. **Catching up hides nothing.** After the stall the feeder sent 930K/s to catch up, and every overdue order keeps its scheduled stamp. Task 13 will show those orders as the latency tail.
4. **The pool replay works:** 3M orders from a 1M pool with 0 errors, so ids stay unique across passes.
5. **The start barrier matters.** Measuring before every stage is ready creates fake latency at the start. Real systems have the same rule: warm up and confirm health first, then measure.

## Run it
```bash
cd ~/lab/c++/low_latency/exchange && make
scripts/run.sh --cpus=0,1,2,3 -- --rate=500000 --duration=5 --progress   # task 12
```
