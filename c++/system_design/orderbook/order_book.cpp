/*
   g++ -std=c++17 -O2 -Wall -Wextra order_book.cpp -o order_book

   Limit order book with price-time priority :
     - The best price matches first, and at the same price the oldest order matches first (FIFO)
     - A trade happens at the resting order's price

   Complexity, L = number of price levels :
     - Add without a trade : O(log L)
     - Each order filled   : O(1), plus O(log L) if its level becomes empty
     - Top of book price   : O(1)

   In an interview, be ready to talk about :
     - cancel and modify, the most frequent operations on a real exchange. See the note in the class.
     - std::map and std::list are clear, but each node is a separate allocation, which means cache misses.
       A low latency book uses pools, flat arrays of price levels when the price range is small, and intrusive lists.
*/

#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <string>

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

    // Size of a price level is the total of all its orders. O(orders in the level), a real book keeps a running total.
    static int64_t level_qty(const Level& lvl) {
        return std::accumulate(lvl.begin(), lvl.end(), int64_t{0},
                               [](int64_t sum, const Order& o) { return sum + o.qty; });
    }

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
        qty = level_qty(bids_.begin()->second);
        return true;
    }
    bool best_ask(int64_t& px, int64_t& qty) const {
        if (asks_.empty()) return false;
        px = asks_.begin()->first;
        qty = level_qty(asks_.begin()->second);
        return true;
    }
    std::string top() const {
        int64_t bp=0,bq=0, ap=0,aq=0;
        bool hb = best_bid(bp,bq), ha = best_ask(ap,aq);
        return (hb ? ("BID " + std::to_string(bp) + " x " + std::to_string(bq)) : "BID -")
               + " | "
               + (ha ? ("ASK " + std::to_string(ap) + " x " + std::to_string(aq)) : "ASK -");
    }

    // Note for future cancel():
    // Keep: std::unordered_map<int orderId, {Side, int64_t price, Level::iterator}>
    //       The side tells which map holds the price. A std::list iterator stays valid when other orders are removed.
    // On add(): store the iterator of the resting order. Remove the entry when the order is fully filled.
    // cancel(id) = find, level.erase(it), erase the price level if it is empty. O(1), plus O(log L) to find the level.
};

static void check_top(const MinimalOrderBook& ob, const std::string& want) {
    std::string got = ob.top();
    std::cout << "TOP: " << got;
    if (got != want) std::cout << "  MISMATCH, expected " << want;
    std::cout << '\n';
}

// --------------------------- tiny demo ---------------------------
int main() {
    MinimalOrderBook ob;

    ob.add({1, Side::Buy , 100, 10});   // bid 100 x10
    ob.add({2, Side::Buy ,  99,  5});   // bid 99 x5
    ob.add({3, Side::Sell, 101,  4});   // ask 101 x4 (no trade)
    check_top(ob, "BID 100 x 10 | ASK 101 x 4");

    ob.add({4, Side::Sell, 100,  6});   // trades 6 @100 vs #1 (remains 4 on #1)
    check_top(ob, "BID 100 x 4 | ASK 101 x 4");

    ob.add({5, Side::Sell,  99, 10});   // fills #1 (4 @100), then #2 (5 @99), leaves 1 @99 resting
    check_top(ob, "BID - | ASK 99 x 1");

    ob.add({6, Side::Buy ,  99,  1});   // trades 1 @99, #3 is still resting at 101
    check_top(ob, "BID - | ASK 101 x 4");

    // FIFO at same price using list:
    ob.add({7, Side::Buy , 100, 5});
    ob.add({8, Side::Buy , 100, 5});    // arrives after #7
    check_top(ob, "BID 100 x 10 | ASK 101 x 4");   // The size of the whole level
    ob.add({9, Side::Sell, 100, 7});    // consumes #7 (5), then 2 from #8
    check_top(ob, "BID 100 x 3 | ASK 101 x 4");

    return 0;
}

