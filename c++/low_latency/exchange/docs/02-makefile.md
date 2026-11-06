# Task 2: Makefile

## Usage
| Command | Result |
|---|---|
| `make` | release build → `build/release/bin/` |
| `make BUILD=debug` | debug build with sanitizers → `build/debug/bin/` |
| `make info` | prints compiler and flags |
| `make clean` | deletes `build/` |

Release and debug live in separate folders, so switching between them never mixes object files.

## Why these flags
| Flag | Why |
|---|---|
| `-O2` | Real optimisation. Benchmarks of `-O0` code are meaningless. (`-O3` is tried later as an experiment, it's not always faster.) |
| `-g` | Debug symbols in the release build, so `perf report` and flame graphs show function names. Symbols don't slow the code down. |
| `-fno-omit-frame-pointer` | Keeps a register pointing to each stack frame, so `perf record -g` can walk call stacks. Costs ~1%. |
| `-march=native` (x86) / `-mcpu=native` (ARM) | Lets the compiler use every instruction the build machine supports (AVX2 etc.). The binary may not run on an older CPU, so build on the machine you benchmark on. |
| `-DNDEBUG` | Disables `assert()` in release. |
| `-O0 -g3` (debug) | No optimisation, full debug info: easiest to step through in gdb. |
| `-fsanitize=address,undefined` (debug) | Catches out-of-bounds access, use-after-free and undefined behaviour at runtime. Never benchmark with these on (2–3× slower). |
| `-pthread` | Threads and shared-memory primitives. |
| `-lrt` (Linux) | `shm_open` lived in librt on older glibc; harmless on new ones. |
| `-MMD -MP` | Compiler writes a `.d` file listing the headers each `.cpp` includes, so editing a header rebuilds exactly the files that use it. |

## Gotcha we hit
Make treats `.o` files made by a chain of pattern rules as *intermediate* and deletes them after linking,
so every `make` rebuilt everything. Fix: a static pattern rule (`$(BINS): $(BIN_DIR)/%: ...`)
names each object explicitly, so make keeps them and rebuilds only what changed.
