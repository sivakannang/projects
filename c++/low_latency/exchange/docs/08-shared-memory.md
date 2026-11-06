# Task 8: Shared memory segment

Files: `common/shm.hpp`, `common/shm.cpp`, `tests/shm_test.cpp`.

## How it works
```
creator                                   attacher
shm_open("/lab_feed", O_CREAT|O_EXCL)     shm_open("/lab_feed", O_RDWR)
ftruncate(fd, 64 + payload)               fstat(fd) → size
mmap(MAP_SHARED) → address A              mmap(MAP_SHARED) → address B   (A != B)
         \                                   /
          same physical pages: /dev/shm/lab_feed (tmpfs, RAM)
```
- After `mmap` there are no system calls: reading and writing are plain loads and stores.
- `shm_open` only returns a file descriptor; `mmap` is always what makes the memory usable.
  - **NSE style:** `shm_open("/name")`, which makes an object in `/dev/shm`.
  - **JPMC style (probably):** `open("/path/file")` then `mmap`. With a path under `/dev/hugepages` that gives huge pages (task 34).
- Each process maps the segment at a different address (the test prints both), so nothing stored inside may be a pointer. Rings use indices.
- The segment outlives every process until `shm_unlink`. On Linux, list leftovers with `ls -l /dev/shm`.

## Segment header (first 64 bytes)
| Field | Purpose |
|---|---|
| `magic` "LABSHM01" | rejects memory that isn't ours |
| `version` | rejects a build with a different layout (bump `kShmVersion` when a payload layout changes) |
| `headerSize`, `totalSize`, `payloadSize` | consistency checks |
| `creatorPid` | who built it (for the monitor and cleanup) |
| `ready` (atomic) | 0 while being built; the creator calls `markReady()` (release store) after building the payload |

`open()` waits for `ready` with an acquire load, up to a timeout. That covers three cases:
- **Creator still between `shm_open` and `ftruncate`:** the size is still 0, so `open()` waits.
- **Creator still building the payload:** `ready` is 0, so `open()` waits.
- **Creator crashed half-way:** `open()` times out with a clear error.

This is the same idea as the ready flag NSE kept in its segment header.

## Lifecycle rules
- `create()` uses `O_EXCL` and **fails if the name exists**, which usually means a leftover from a crashed run.
  `create(..., replace=true)` unlinks the old name first. Processes still attached keep the old memory; the test checks that the old mapping stays valid.
- The destructor only unmaps. **Only the owner calls `unlink()`**, at shutdown.
- Names: `/name`, at most 31 characters (macOS limit, enforced everywhere for portability).
- No pre-touching yet: pages are allocated on first write (page faults). Task 34 adds `MAP_POPULATE`, pre-touching and huge pages as switches.

## Test
The parent creates the segment, writes a pattern and calls `markReady()`, then `fork()`s. The child opens the
segment by name, verifies the pattern and writes a reply, and the parent verifies the reply.
Failure cases checked:
- create twice (refused) and replace (works)
- open before ready (times out)
- bad magic, wrong version
- open after unlink, unlink twice, bad name

Result: passes on the Mac and on the Dell in release, debug (ASan/UBSan) and TSan builds,
and `/dev/shm` is empty afterwards.
