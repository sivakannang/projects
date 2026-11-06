// Prints the byte layout of every message struct and checks there are no hidden padding bytes:
// the sum of the declared field sizes (including explicit pads) must equal sizeof(struct).

#include <cstddef>
#include <cstdio>

#include "messages.hpp"

using namespace lab;

static int failures = 0;

struct LayoutPrinter {
    const char* name;
    size_t size;
    size_t declared = 0;

    LayoutPrinter(const char* n, size_t s, size_t align) : name(n), size(s) {
        std::printf("\n%s  (sizeof %zu, alignof %zu)\n", name, size, align);
        std::printf("  %-14s %6s %5s\n", "field", "offset", "size");
    }
    void field(const char* f, size_t offset, size_t sz) {
        std::printf("  %-14s %6zu %5zu\n", f, offset, sz);
        declared += sz;
    }
    ~LayoutPrinter() {
        bool ok = declared == size;
        std::printf("  declared %zu bytes, sizeof %zu -> %s\n", declared, size,
                    ok ? "no hidden padding" : "HIDDEN PADDING");
        if (!ok) ++failures;
    }
};

#define LAYOUT(T) LayoutPrinter p_##T(#T, sizeof(T), alignof(T))
#define FIELD(T, f) p_##T.field(#f, offsetof(T, f), sizeof(T::f))

int main() {
    {
        LAYOUT(MsgHeader);
        FIELD(MsgHeader, seq);
        FIELD(MsgHeader, tsFeeder);
        FIELD(MsgHeader, tsReceiver);
        FIELD(MsgHeader, tsProcessor);
        FIELD(MsgHeader, tsSender);
        FIELD(MsgHeader, type);
        FIELD(MsgHeader, pad);
    }
    {
        LAYOUT(NewOrder);
        FIELD(NewOrder, clientOrderId);
        FIELD(NewOrder, price);
        FIELD(NewOrder, qty);
        FIELD(NewOrder, instrument);
        FIELD(NewOrder, account);
        FIELD(NewOrder, side);
        FIELD(NewOrder, ordType);
        FIELD(NewOrder, tif);
        FIELD(NewOrder, pad);
    }
    {
        LAYOUT(ModifyOrder);
        FIELD(ModifyOrder, clientOrderId);
        FIELD(ModifyOrder, newPrice);
        FIELD(ModifyOrder, newQty);
        FIELD(ModifyOrder, instrument);
        FIELD(ModifyOrder, account);
        FIELD(ModifyOrder, pad);
    }
    {
        LAYOUT(CancelOrder);
        FIELD(CancelOrder, clientOrderId);
        FIELD(CancelOrder, instrument);
        FIELD(CancelOrder, account);
        FIELD(CancelOrder, pad);
    }
    {
        LAYOUT(ExecReport);
        FIELD(ExecReport, clientOrderId);
        FIELD(ExecReport, lastPx);
        FIELD(ExecReport, exchOrderNo);
        FIELD(ExecReport, filledQty);
        FIELD(ExecReport, leavesQty);
        FIELD(ExecReport, status);
        FIELD(ExecReport, reject);
        FIELD(ExecReport, tradeCount);
    }
    {
        LAYOUT(Msg);
        FIELD(Msg, hdr);
        p_Msg.field("body (union)", offsetof(Msg, newOrder), sizeof(NewOrder));
    }

    // Msg{} must zero every byte, including the explicit pads
    Msg m{};
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&m);
    for (size_t i = 0; i < sizeof m; ++i)
        if (bytes[i] != 0) {
            std::printf("\nMsg{} left byte %zu non-zero\n", i);
            ++failures;
        }

    std::printf("\n%s\n", failures ? "FAILED" : "all layout checks passed");
    return failures ? 1 : 0;
}
