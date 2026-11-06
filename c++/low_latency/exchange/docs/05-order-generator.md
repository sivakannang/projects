# Task 5: Order generator

| File | What it does |
|---|---|
| `common/rng.hpp` | `Rng`: splitmix64 random numbers + our own range mapping |
| `common/instruments.hpp` | 16 futures-like instruments: reference price, tick, lot, max qty, price band |
| `common/order_gen.hpp/.cpp` | `GenConfig` (the mix) and `OrderGenerator` |
| `common/msg_format.hpp` | One-line readable form of any `Msg` (tests / debug only) |
| `tests/order_gen_test.cpp` | Sample dump, mix check, determinism check, speed |

## Design
- **Generate up front, replay while sending.** The feeder's send loop then only copies 64 bytes per
  order, so the generator's cost and jitter never enter the measurement. Same idea as NSE replaying
  production data. For long runs the feeder loops over the flow and adds an offset to every
  `clientOrderId` so ids stay unique (`lastOrderId()` gives the offset).
- **Own random generator, not `std::` distributions.** libc++ (Mac) and libstdc++ (Linux) implement
  `std::uniform_int_distribution` differently, so the same seed gives different streams per machine.
  splitmix64 + multiply-shift mapping gives the identical stream everywhere.
- **Modify / Cancel target orders we created** (Limit Day, valid ones). The generator can't see
  fills, so some of them will hit already-filled orders → the engine answers `UnknownOrder`.
  Realistic: a cancel can race a fill.
- **Each invalid order has exactly one error**, cycling through the validator's checks.

## Default mix (all configurable)
| Setting | Default |
|---|---|
| New / Modify / Cancel | 70 / 10 / 20 % |
| Of new: Market / IOC / rest Limit Day | 5 / 15 / 80 % |
| Aggressive Limit Day (priced to cross) | 30 % |
| Invalid new orders | 2 % |
| Hot instruments | first 2 of 16 get 50 % of the flow |
| Prices | passive: 1–10 ticks away from reference; aggressive: cross by 1–3 ticks |
| Modify kinds | 50 % qty down (keeps priority), 25 % qty up, 25 % new price (both lose priority) |

## Results
| | Mac (M-series) | Dell (i3-4005U) |
|---|---|---|
| Checksum, seed 42, 1M msgs | `c6cbafa6e3c153e0` | `c6cbafa6e3c153e0` (identical) |
| Generation speed (release) | 41 ns/msg | 113 ns/msg |
| Every mix figure within | ±0.1 % of configured | ±0.1 % of configured |

At NSE-like rates (~300K orders/s = 3,300 ns per order) even generating on the fly would fit,
but replaying keeps the send loop's cost constant and tiny, which matters for latency jitter.
