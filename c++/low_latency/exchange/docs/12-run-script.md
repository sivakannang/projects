# Task 12: Run script (start, watch, stop)

Files: `scripts/run.sh`, `tests/pipeline_test.sh` (now a wrapper), `CLAUDE.md`, `runs/` (git-ignored).

```bash
scripts/run.sh --cpus=0,1,2,3 -- --rate=500000 --duration=10 --progress
scripts/run.sh -- --count=2000000          # unpinned, as fast as possible
scripts/run.sh --clean                     # kill stray lab programs, remove leftover rings
```
Options: `--cpus=F,R,P,S`, `--capacity`, `--bin`, `--out`, `--timeout`, `--clean`. Everything after `--` goes to the feeder.

## What it does
1. **Checks before starting.** Some refuse to run, some only warn:
   - **Refuses:**
     - the programs aren't built;
     - lab programs are already running. They are matched by executable path, so a real `sender` on a shared host is never touched;
     - a ring in `/dev/shm` is held by a live process;
     - a bad `--cpus` list.
   - **Removes:** leftover rings whose creator is dead. The creator's pid is read from the shm header at offset 32.
   - **Warns:**
     - two programs on hyper-threads of one core;
     - CPUs on different NUMA nodes;
     - a chosen CPU more than 20% busy in a 1 s sample before the run;
     - a governor that isn't `performance`.
2. **Start:** stages first, then the feeder, each with `--cpu=`. Each program's output goes to its own log, and the feeder's log is also shown live.
3. **Watch** every 0.2 s:
   - **A program exits with an error:** stop the others. Before this script, a feeder failure left three stages spinning for 30 s.
   - **The feeder finishes but a stage is still running 3 s later:** stop it.
   - **`--timeout` passes:** stop everything.
   - **Ctrl-C / SIGTERM:** stop everything.
   - **"Stop"** means SIGTERM first: each program exits through its stop flag and unlinks its ring. Any still running 3 s later get SIGKILL.
4. **Summary:** exit code and last line per program. Leftover rings after a clean run count as a failure; after a kill they are expected, and removed. Exit 0 only if all four exit 0.

## Run folder: `runs/<date_time>/`, plus a `runs/latest` link
| File | Contents |
|---|---|
| `command.txt` | the exact command |
| `env.txt` | code version (git hash + "uncommitted changes", or "no git"), build, OS, CPU model, `lscpu` (sockets, cores, threads, NUMA), governor, isolated CPUs, kernel boot line (`isolcpus`, `nohz_full`), tuned profile, `perf_event_paranoid`, and each program's CPU with its socket/core/node |
| `feeder.log` … `sender.log` | each program's output |

Every number stays tied to the code and machine that produced it. Tasks 13–16 add their latency and CSV results to the same folder.

## Tested
| Case | Mac | Dell |
|---|---|---|
| Normal run, all exit 0 | ✓ | ✓ (pinned 0,1,2,3: warns that 0/2 and 1/3 are hyper-thread pairs) |
| Feeder bad option: others stopped, exit 1 | ✓ (a stage still waiting for its ring shows "killed by signal 15") | |
| Ctrl-C in a terminal, SIGTERM to the script | ✓ (all exit through the stop flag) | |
| `--timeout` | ✓ | |
| Stray program, then `--clean` | ✓ | ✓ |
| Ring left by a killed program | | ✓ (removed before the start) |
| `--cpus` with too few CPUs, a duplicate, or a CPU that doesn't exist | ✓ | ✓ |
| `make test` (uses the script) | ✓ | ✓ |

## Lessons
1. **The script is part of the measurement.** Leftover rings, stray spinners and unrecorded settings make a number meaningless. The run folder is what lets you say later *"this was commit X, CPUs 10–13 on node 0, performance governor"*.
2. **On a big shared host, CPU choice is a correctness issue:**
   - same NUMA node;
   - no hyper-thread siblings;
   - cores nobody else is using;
   - ideally cores in `isolcpus`.

   `env.txt` shows the layout, and the warnings catch mistakes.
3. **Background programs started by a script ignore SIGINT** until they install their own handler. A non-interactive shell starts them that way, so the script must forward the stop itself (SIGTERM). That's also why a stage still waiting to connect dies by signal 15 instead of exiting cleanly.
4. **Programs are matched by executable path, never by name alone.** `pkill sender` on a UAT host could hit someone else's process.

## Office UAT host (32–64 cores)
```bash
lscpu -e                                   # CPU, NODE, SOCKET, CORE columns: pick 4 CPUs on one node, distinct cores
cat /sys/devices/system/cpu/isolated       # isolated CPUs are the best choice if the owners allow it
make && make test
scripts/run.sh --cpus=10,12,14,16 -- --rate=1000000 --duration=10 --progress
```
