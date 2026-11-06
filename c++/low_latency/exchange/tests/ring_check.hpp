#pragma once
// Shared by the ring tests: message content is a function of seq, so the consumer can verify
// every message without sharing any state with the producer (works across processes too).

#include <cstring>

#include "messages.hpp"

namespace lab::test {

inline void fill(Msg& m, uint64_t seq) {
    m = Msg{};
    m.hdr.seq = seq;
    m.hdr.type = MsgType::NewOrder;
    m.newOrder.clientOrderId = seq * 7919;
    m.newOrder.price = int64_t(seq) * 31 - 17;
    m.newOrder.qty = uint32_t(seq ^ 0x5a5a5a5a);
}

inline bool matches(const Msg& m, uint64_t seq) {
    Msg expected;
    fill(expected, seq);
    return std::memcmp(&m, &expected, sizeof(Msg)) == 0;
}

}  // namespace lab::test
