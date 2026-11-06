# Task 4: Message structs

File: `common/messages.hpp`. Layout printer: `tests/msg_layout.cpp` (`make test`).

## Shape
Every ring slot holds one `Msg` of 64 bytes:

```
Msg (64)
├── MsgHeader (32)   seq, tsFeeder, tsReceiver/tsProcessor/tsSender, type, pad[3]
└── union body (32)  NewOrder | ModifyOrder | CancelOrder | ExecReport
```

A tagged union (header `type` says which body is valid) is the classic C protocol style,
same idea as the NSE C structs. One fixed size means a ring slot can hold any message.

| Type | Direction | Key fields |
|---|---|---|
| NewOrder | in | clientOrderId, price, qty, instrument, account, side, ordType, tif |
| ModifyOrder | in | clientOrderId, newPrice, newQty |
| CancelOrder | in | clientOrderId |
| ExecReport | out | status, reject reason, exchOrderNo, filledQty, leavesQty, lastPx, tradeCount |
| TradeReport | out | reserved for per-fill confirmations later |

ExecStatus: Accepted, PartFill, Filled, Expired (IOC/Market remainder cancelled), Modified,
Cancelled, Rejected.

## Rules and why
| Rule | Why |
|---|---|
| Fixed size, trivially copyable, no pointers / std::string | Lives in shared memory; each process maps it at a different address, so a pointer from one process is garbage in another |
| Prices in integer paise | `double` can't hold 0.05 exactly; integer compare is exact and fast |
| Enums are `uint8_t` | 1 byte each instead of 4 |
| Fields large → small | No hidden compiler padding |
| Explicit `pad` bytes | Layout is visible; `Msg{}` zeroes them, so no garbage bytes (memcmp, checksums, replay files all stay deterministic) |
| `static_assert` on sizes and offsets | Any accidental layout change fails to compile |
| Baseline alignment = 8 (natural), no `alignas(64)`, no `#pragma pack` | Alignment and packing are measured later as experiments, one change at a time |

## What field order does (example, not in the code)
```cpp
struct Bad  { uint8_t side; uint64_t id; uint8_t tif; uint64_t price; };  // sizeof 32: 14 hidden pad bytes
struct Good { uint64_t id; uint64_t price; uint8_t side; uint8_t tif; };  // sizeof 24: 6 pad bytes at the end
```
Each `uint64_t` must start at a multiple of 8, so a 1-byte field in front of it wastes 7 bytes.

## Compromise we made
The ExecReport body has to fit 32 bytes, so `exchOrderNo` is 32-bit (4 billion orders,
plenty for a lab run; a real exchange would use 64 bits).

## Verified
`msg_layout` on Mac (clang) and Dell (g++ 15), release and debug:
every struct's declared fields add up to its `sizeof` → no hidden padding; `Msg{}` zeroes all 64 bytes.
