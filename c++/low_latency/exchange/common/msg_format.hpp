#pragma once
// Human-readable one-line form of a Msg, for tests and debug dumps (never the hot path).

#include <cstdio>
#include <string>

#include "messages.hpp"

namespace lab {

inline std::string formatPrice(int64_t paise) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%lld.%02lld", (long long)(paise / 100), (long long)(paise % 100));
    return buf;
}

inline std::string formatMsg(const Msg& m) {
    char buf[160];
    switch (m.hdr.type) {
        case MsgType::NewOrder: {
            const NewOrder& o = m.newOrder;
            const char* kind = o.ordType == OrdType::Market ? "MKT" : (o.tif == Tif::Ioc ? "IOC" : "DAY");
            std::snprintf(buf, sizeof buf, "New    id=%-6llu ins=%-2u acct=%-4u %-4s %s qty=%-5u px=%s",
                          (unsigned long long)o.clientOrderId, o.instrument, o.account,
                          o.side == Side::Buy ? "BUY" : "SELL", kind, o.qty,
                          o.ordType == OrdType::Market ? "-" : formatPrice(o.price).c_str());
            break;
        }
        case MsgType::ModifyOrder: {
            const ModifyOrder& o = m.modify;
            std::snprintf(buf, sizeof buf, "Modify id=%-6llu ins=%-2u acct=%-4u newQty=%-5u newPx=%s",
                          (unsigned long long)o.clientOrderId, o.instrument, o.account, o.newQty,
                          formatPrice(o.newPrice).c_str());
            break;
        }
        case MsgType::CancelOrder: {
            const CancelOrder& o = m.cancel;
            std::snprintf(buf, sizeof buf, "Cancel id=%-6llu ins=%-2u acct=%-4u", (unsigned long long)o.clientOrderId,
                          o.instrument, o.account);
            break;
        }
        case MsgType::ExecReport: {
            const ExecReport& e = m.exec;
            std::snprintf(buf, sizeof buf, "Exec   id=%-6llu %-9s filled=%-5u leaves=%-5u trades=%u lastPx=%s reject=%u",
                          (unsigned long long)e.clientOrderId, toString(e.status), e.filledQty, e.leavesQty,
                          unsigned(e.tradeCount), formatPrice(e.lastPx).c_str(), unsigned(e.reject));
            break;
        }
        default:
            std::snprintf(buf, sizeof buf, "%s", toString(m.hdr.type));
    }
    return buf;
}

}  // namespace lab
