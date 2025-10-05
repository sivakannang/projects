#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <cstdint>

enum class Side { Buy, Sell };

struct Order {
    int id;
    Side side;
    int64_t price; // integer price (e.g., paise)
    int64_t qty;
};

class MinimalOrderBook {
    // Bids: highest price first; Asks: lowest price first
    using Level = std::list<Order>;
    std::map<int64_t, Level, std::greater<int64_t>> bids_;
    std::map<int64_t, Level>                        asks_;

    static void trade_print(int64_t px, int64_t qty, int buy_id, int sell_id) {
        std::cout << "TRADE px=" << px << " qty=" << qty
                  << " buy#" << buy_id << " sell#" << sell_id << '\n';
    }

    void match_buy(Order& o) {
        auto it = asks_.begin(); // best ask (lowest price)
        while (o.qty > 0 && it != asks_.end() && it->first <= o.price) {
            auto& lvl = it->second;
            while (o.qty > 0 && !lvl.empty()) {
                Order& top = lvl.front();
                const int64_t exec = std::min(o.qty, top.qty);
                trade_print(it->first, exec, o.id, top.id);
                o.qty     -= exec;
                top.qty   -= exec;
                if (top.qty == 0) lvl.pop_front();
            }
            if (lvl.empty()) it = asks_.erase(it);
            else break;
        }
        if (o.qty > 0) bids_[o.price].push_back(o); // rest remaining (FIFO at price)
    }

    void match_sell(Order& o) {
        auto it = bids_.begin(); // best bid (highest price)
        while (o.qty > 0 && it != bids_.end() && it->first >= o.price) {
            auto& lvl = it->second;
            while (o.qty > 0 && !lvl.empty()) {
                Order& top = lvl.front();
                const int64_t exec = std::min(o.qty, top.qty);
                trade_print(it->first, exec, top.id, o.id);
                o.qty     -= exec;
                top.qty   -= exec;
                if (top.qty == 0) lvl.pop_front();
            }
            if (lvl.empty()) it = bids_.erase(it);
            else break;
        }
        if (o.qty > 0) asks_[o.price].push_back(o); // rest remaining (FIFO at price)
    }

public:
    void add(const Order& ord) {
        Order o = ord; // mutable copy (qty will be reduced during matching)
        if (o.side == Side::Buy)  match_buy(o);
        else                      match_sell(o);
    }

    // Top-of-book helpers (minimal)
    bool best_bid(int64_t& px, int64_t& qty) const {
        if (bids_.empty()) return false;
        px = bids_.begin()->first;
        const auto& lvl = bids_.begin()->second;
        qty = lvl.empty() ? 0 : lvl.front().qty;
        return true;
    }
    bool best_ask(int64_t& px, int64_t& qty) const {
        if (asks_.empty()) return false;
        px = asks_.begin()->first;
        const auto& lvl = asks_.begin()->second;
        qty = lvl.empty() ? 0 : lvl.front().qty;
        return true;
    }
    void print_top() const {
        int64_t bp=0,bq=0, ap=0,aq=0;
        bool hb = best_bid(bp,bq), ha = best_ask(ap,aq);
        std::cout << "TOP: "
                  << (hb ? ("BID " + std::to_string(bp) + " x " + std::to_string(bq)) : "BID -")
                  << " | "
                  << (ha ? ("ASK " + std::to_string(ap) + " x " + std::to_string(aq)) : "ASK -")
                  << '\n';
    }

    // Note for future cancel():
    // Keep: std::unordered_map<int /*orderId*/, std::pair<int64_t /*price*/, Level::iterator>>
    // On add(): store iterator returned by push_back; cancel(id) = find, level.erase(it), cleanup price if empty.
};

// --------------------------- tiny demo ---------------------------
int main() {
    MinimalOrderBook ob;

    ob.add({1, Side::Buy , 100, 10});   // bid 100 x10
    ob.add({2, Side::Buy ,  99,  5});   // bid 99 x5
    ob.add({3, Side::Sell, 101,  4});   // ask 101 x4 (no trade)
    ob.print_top();                     // BID 100 x10 | ASK 101 x4

    ob.add({4, Side::Sell, 100,  6});   // trades 6 @100 vs #1 (remains 4 on #1)
    ob.print_top();                     // BID 100 x4 | ASK 101 x4

    ob.add({5, Side::Sell,  99, 10});   // fills #1 (4 @100), then #2 (5 @99), leaves 1 @99 resting
    ob.print_top();                     // BID - | ASK 99 x1

    ob.add({6, Side::Buy ,  99,  1});   // trades 1 @99
    ob.print_top();                     // empty book

    // FIFO at same price using list:
    ob.add({7, Side::Buy , 100, 5});
    ob.add({8, Side::Buy , 100, 5});    // arrives after #7
    ob.add({9, Side::Sell, 100, 7});    // consumes #7 (5), then 2 from #8
    ob.print_top();                     // BID 100 x3 | ASK -

    return 0;
}

