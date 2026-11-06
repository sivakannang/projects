#pragma once
// Messages carried by the rings: feeder -> receiver -> processor -> sender -> feeder.
//
// Every message is one 64 byte Msg = 32 byte header + 32 byte body (a union, one per type).
// Rules, because these live in shared memory mapped at different addresses per process:
//   - fixed size, trivially copyable, no pointers, no std::string
//   - prices are integer paise (never double), enums are 1 byte
//   - fields ordered large -> small; every padding byte is an explicit, zeroed `pad`
//   - layout is checked at compile time by the static_asserts at the bottom
//
// Baseline: natural alignment (alignof == 8), no alignas(64), no #pragma pack.
// Cache-line alignment and packing are measured later as separate experiments.

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace lab {

enum class MsgType : uint8_t {
    NewOrder = 1,
    ModifyOrder = 2,
    CancelOrder = 3,
    ExecReport = 4,
    TradeReport = 5,  // reserved: per-fill confirmations, added later

    // Control: travels through the rings like any message, so it can't overtake real ones
    EndOfStream = 100,  // feeder sent its last message; each stage passes it on and exits
    Ping = 101,         // passed on untouched; the feeder's round trip proves every stage is connected
};

enum class Side : uint8_t { Buy = 1, Sell = 2 };
enum class OrdType : uint8_t { Limit = 1, Market = 2 };
enum class Tif : uint8_t { Day = 1, Ioc = 2 };

enum class ExecStatus : uint8_t {
    Accepted = 1,   // rested in the book, nothing filled
    PartFill = 2,   // some filled, the rest rests in the book
    Filled = 3,     // fully filled
    Expired = 4,    // IOC / Market: unfilled remainder cancelled (may have filled some)
    Modified = 5,
    Cancelled = 6,
    Rejected = 7,   // see RejectReason
};

enum class RejectReason : uint8_t {
    None = 0,
    BadMsgType,
    BadInstrument,
    BadAccount,
    BadSide,
    BadOrdType,
    BadTif,
    BadQty,
    BadLot,
    BadPrice,
    BadTick,
    PriceBand,      // limit price outside the allowed band around the reference price
    NoLiquidity,    // market order with nothing on the other side within protection
    UnknownOrder,   // modify / cancel of an order not in the book
    DuplicateOrder, // clientOrderId already used
};

// 32 bytes, common to every message
struct MsgHeader {
    uint64_t seq;          // feeder sequence number: detects loss and reordering
    uint64_t tsFeeder;     // timestamp when the feeder sent it (ticks)
    uint32_t tsReceiver;   // ticks since tsFeeder when the receiver handled it
    uint32_t tsProcessor;  // ... the processor
    uint32_t tsSender;     // ... the sender
    MsgType type;
    uint8_t pad[3];
};

// 32 byte bodies
struct NewOrder {
    uint64_t clientOrderId;  // unique per order, chosen by the feeder
    int64_t price;           // paise; 0 for Market
    uint32_t qty;
    uint32_t instrument;
    uint32_t account;
    Side side;
    OrdType ordType;
    Tif tif;
    uint8_t pad[1];
};

struct ModifyOrder {
    uint64_t clientOrderId;  // order to modify
    int64_t newPrice;
    uint32_t newQty;
    uint32_t instrument;
    uint32_t account;
    uint8_t pad[4];
};

struct CancelOrder {
    uint64_t clientOrderId;  // order to cancel
    uint32_t instrument;
    uint32_t account;
    uint8_t pad[16];
};

struct ExecReport {
    uint64_t clientOrderId;
    int64_t lastPx;        // price of the last fill, 0 if none
    uint32_t exchOrderNo;  // assigned by the engine (32 bits is plenty for a lab run)
    uint32_t filledQty;    // quantity filled by this request
    uint32_t leavesQty;    // quantity still resting in the book
    ExecStatus status;
    RejectReason reject;
    uint16_t tradeCount;   // number of fills this request produced
};

// One ring slot
struct Msg {
    MsgHeader hdr;
    union {
        NewOrder newOrder;
        ModifyOrder modify;
        CancelOrder cancel;
        ExecReport exec;
    };
};

// ---- compile-time layout checks -------------------------------------------------------
static_assert(sizeof(MsgHeader) == 32);
static_assert(sizeof(NewOrder) == 32);
static_assert(sizeof(ModifyOrder) == 32);
static_assert(sizeof(CancelOrder) == 32);
static_assert(sizeof(ExecReport) == 32);
static_assert(sizeof(Msg) == 64);
static_assert(alignof(Msg) == 8, "baseline uses natural alignment, not alignas(64)");

static_assert(offsetof(Msg, hdr) == 0);
static_assert(offsetof(Msg, newOrder) == 32);
static_assert(offsetof(MsgHeader, tsFeeder) == 8);
static_assert(offsetof(MsgHeader, type) == 28);
static_assert(offsetof(NewOrder, price) == 8);
static_assert(offsetof(NewOrder, side) == 28);
static_assert(offsetof(ExecReport, status) == 28);

static_assert(std::is_trivially_copyable_v<Msg>, "must be safe to memcpy / place in shared memory");
static_assert(std::is_standard_layout_v<Msg>, "C-compatible layout, offsetof is well defined");

// ---- names for logs and tests ---------------------------------------------------------
inline const char* toString(MsgType t) {
    switch (t) {
        case MsgType::NewOrder: return "NewOrder";
        case MsgType::ModifyOrder: return "ModifyOrder";
        case MsgType::CancelOrder: return "CancelOrder";
        case MsgType::ExecReport: return "ExecReport";
        case MsgType::TradeReport: return "TradeReport";
        case MsgType::EndOfStream: return "EndOfStream";
        case MsgType::Ping: return "Ping";
    }
    return "?";
}

inline const char* toString(ExecStatus s) {
    switch (s) {
        case ExecStatus::Accepted: return "Accepted";
        case ExecStatus::PartFill: return "PartFill";
        case ExecStatus::Filled: return "Filled";
        case ExecStatus::Expired: return "Expired";
        case ExecStatus::Modified: return "Modified";
        case ExecStatus::Cancelled: return "Cancelled";
        case ExecStatus::Rejected: return "Rejected";
    }
    return "?";
}

}  // namespace lab
