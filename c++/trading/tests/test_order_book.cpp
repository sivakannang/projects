#include <gtest/gtest.h>
#include "order_book.hpp"

using namespace ob;

static std::uint64_t g_ts = 0;
static Order O(OrderId id, Side s, Type t, TIF tif, Price px, Qty q) {
	return Order{ id, s, t, tif, px, q, ++g_ts };
}

TEST(OrderBook, SimpleCross_TradesAtMakerPrice) {
	Book b;
	b.submit(O(1, Side::Buy,  Type::Limit, TIF::GFD, 100, 10)); // rest
	b.clearTrades();

	b.submit(O(2, Side::Sell, Type::Limit, TIF::GFD,  99,  6)); // cross
	ASSERT_EQ(b.trades().size(), 1u);
	EXPECT_EQ(b.trades()[0].maker, 1u);
	EXPECT_EQ(b.trades()[0].taker, 2u);
	EXPECT_EQ(b.trades()[0].px, 100);
	EXPECT_EQ(b.trades()[0].qty, 6);

	// Remaining qty at bid 100 is 4
	EXPECT_EQ(b.totalQtyAt(Side::Buy, 100), 4);
	EXPECT_TRUE(b.hasBestBid());
	EXPECT_EQ(b.bestBid(), 100);
}

TEST(OrderBook, FIFOWithinPriceLevel) {
	Book b;
	b.submit(O(10, Side::Sell, Type::Limit, TIF::GFD, 105, 5)); // rests first
	b.submit(O(11, Side::Sell, Type::Limit, TIF::GFD, 105, 7)); // rests second
	b.clearTrades();

	// Buy crosses 105 for qty=8 → should first fill id=10 (5), then id=11 (3)
	b.submit(O(12, Side::Buy, Type::Limit, TIF::GFD, 105, 8));
	ASSERT_EQ(b.trades().size(), 2u);
	EXPECT_EQ(b.trades()[0].maker, 10u);
	EXPECT_EQ(b.trades()[0].qty, 5);
	EXPECT_EQ(b.trades()[1].maker, 11u);
	EXPECT_EQ(b.trades()[1].qty, 3);

	// Remaining at 105 = 4 (from id=11)
	EXPECT_EQ(b.totalQtyAt(Side::Sell, 105), 4);
}

TEST(OrderBook, IOCDoesNotRest) {
	Book b;
	b.submit(O(20, Side::Sell, Type::Limit, TIF::GFD, 101, 5)); // rest ask
	b.clearTrades();

	// IOC buy 101 for 10 → fills 5 and discards remainder
	b.submit(O(21, Side::Buy, Type::Limit, TIF::IOC, 101, 10));
	ASSERT_EQ(b.trades().size(), 1u);
	EXPECT_EQ(b.trades()[0].qty, 5);
	// No resting IOC leftover; asks now empty
	EXPECT_EQ(b.numAskLevels(), 0u);
	EXPECT_FALSE(b.hasBestAsk());
}

TEST(OrderBook, MarketFillsThenCancelsLeftover) {
	Book b;
	b.submit(O(30, Side::Buy,  Type::Limit,  TIF::GFD, 100, 6));
	b.submit(O(31, Side::Buy,  Type::Limit,  TIF::GFD,  99, 7));
	b.clearTrades();

	// MARKET sell for 10: fills 6@100 then 4@99; no leftover rests
	b.submit(O(32, Side::Sell, Type::Market, TIF::IOC,   0, 10));
	ASSERT_EQ(b.trades().size(), 2u);
	EXPECT_EQ(b.trades()[0].px, 100);
	EXPECT_EQ(b.trades()[0].qty, 6);
	EXPECT_EQ(b.trades()[1].px,  99);
	EXPECT_EQ(b.trades()[1].qty, 4);

	// Remaining bid depth at 99 is 3
	EXPECT_EQ(b.totalQtyAt(Side::Buy, 99), 3);
}

TEST(OrderBook, CancelById) {
	Book b;
	b.submit(O(40, Side::Buy, Type::Limit, TIF::GFD, 100, 7));
	b.submit(O(41, Side::Buy, Type::Limit, TIF::GFD,  99, 2));

	EXPECT_TRUE(b.hasOrder(40));
	EXPECT_TRUE(b.cancel(40));
	EXPECT_FALSE(b.hasOrder(40));

	// Cancel non-existent
	EXPECT_FALSE(b.cancel(9999));
}

TEST(OrderBook, DoNotRestIfStillCrossing) {
	Book b;
	b.submit(O(50, Side::Sell, Type::Limit, TIF::GFD, 100, 5)); // best ask 100
	b.clearTrades();

	// Limit BUY 105 should fully cross; any leftover must not rest if still crossing
	b.submit(O(51, Side::Buy, Type::Limit, TIF::GFD, 105, 10));
	// We only had 5 on ask side; leftover 5 should rest at 105 since no longer crossing.
	EXPECT_EQ(b.totalQtyAt(Side::Buy, 105), 5);

	// Now add an ask 104 — BUY 105 *would* cross, but it's already resting at 105.
	// Safety in submit() prevents resting a crossing order at the time of rest only (correct behavior).
	b.clearTrades();
	b.submit(O(52, Side::Sell, Type::Limit, TIF::GFD, 104, 3));
	// That should trade immediately against resting 105 bid
	ASSERT_EQ(b.trades().size(), 1u);
	EXPECT_EQ(b.trades()[0].px, 105);
	EXPECT_EQ(b.totalQtyAt(Side::Buy, 105), 2);
}

