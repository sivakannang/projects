# Exchange lab — task plan

Goal: build a small exchange pipeline (feeder → receiver → processor → sender → feeder) over
shared-memory SPSC ring buffers, with a new matching engine, and use it to learn benchmarking,
latency, throughput and optimisation step by step. Each task is discussed first, then built,
then measured. Notes for each task go in `docs/`.

Order scope: Limit Day, Limit IOC, Market (with price protection), Cancel, Modify.
Price-time priority, trade at the resting order's price, one book per instrument.

## Phase A: Skeleton
- [x] 1. Folders and the 5 programs, each printing its name
- [x] 2. Makefile: build all, debug vs release, `make clean`
- [x] 3. Common helpers: command-line options, logging, CPU pinning

## Phase B: Messages
- [x] 4. Message structs (New / Modify / Cancel, execution report), 64 bytes, per-hop timestamps
- [x] 5. Order generator: deterministic flow, configurable mix

## Phase C: Ring buffer (in one process)
- [x] 6. Simplest SPSC ring buffer + two-thread test (no loss, no reordering)
- [x] 7. Timing helpers (rdtsc + calibration) and first ring throughput/latency benchmark

## Phase D: Shared memory
- [x] 8. Shared-memory wrapper: create, attach, clean up (shm_open + mmap)
- [x] 9. Ring buffer in shared memory, one writer process + one reader process

## Phase E: First pipeline (processor passes messages through)
- [x] 10. feeder → receiver → processor → sender → feeder, every message arrives in order
- [x] 11. Feeder rate control: `--rate=50000 --duration=10`
- [x] 12. Start/stop script: create shm, start processes pinned, clean shutdown

## Phase F: Measurement
- [ ] 13. End-to-end latency from timestamps: p50/p90/p99/p99.9/max
- [ ] 14. Per-stage counters in shm: processed, queue depth, full/empty counts
- [ ] 15. Monitor tool: per-second messages/sec, queue depth, latency per stage
- [ ] 16. Capacity test: step the rate up until latency or queues blow up, save CSV
- [ ] 17. Processor dummy mode `--work=spin:<ns>` (reproduce the 270K → 330K arithmetic)

## Phase G: Matching engine (new)
- [ ] 18. Discussion: price levels, order-id lookup, preallocated order pool
- [ ] 19. Order pool and per-price FIFO queues
- [ ] 20. Limit Day: add + match, price-time priority, unit tests
- [ ] 21. IOC, and Market with price protection
- [ ] 22. Cancel
- [ ] 23. Modify with priority rules
- [ ] 24. Validator
- [ ] 25. Plug engine into processor: `--work=match`

## Phase H: Baseline
- [ ] 26. Baseline report: capacity curve, latency at several rates, perf stat per process, flame graph

## Phase I: Optimisations (one at a time, before/after numbers)
- [ ] 27. Ring buffer versions: mutex → atomic → cache-aligned → cached indices
- [ ] 28. Waiting strategy: busy-spin vs pause vs yield vs sleep
- [ ] 29. Core pinning: same core vs different cores; P-core vs E-core (HP)
- [ ] 30. NSE change 2: copy out of the ring vs process in place
- [ ] 31. NSE change 1: move validation from processor to receiver
- [ ] 32. Batching and prefetching
- [ ] 33. Order book data structure tuning
- [ ] 34. Memory: huge pages, mlock, pre-touching pages
- [ ] 35. OS noise: isolated cores, no desktop
- [ ] 36. UDP feeder → receiver vs shared memory (cost of the kernel)
- [ ] 37. Final report and interview notes

## Optional
- [ ] 38. Asynchronous binary logger (hot path writes a record into a ring buffer, a background thread formats and writes it)
