#pragma once
#include <cstdint>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <functional>
#include <optional>

namespace ob {

	using OrderId = std::uint64_t;
	using Price   = std::int64_t;   // price in integer ticks
	using Qty     = std::int64_t;

	enum class Side { Buy, Sell };
	enum class Type { Limit, Market };
	enum class TIF  { GFD, IOC };

	struct Order {
		OrderId id{};
		Side    side{};
		Type    type{};
		TIF     tif{};
		Price   px{};     // ignored for Market
		Qty     qty{};    // open qty
		std::uint64_t ts{}; // optional monotonic timestamp to break ties
	};

	struct Trade {
		OrderId maker{};
		OrderId taker{};
		Price   px{};
		Qty     qty{};
	};

	// Price → FIFO queue (stable iterators)
	using PriceLevelQ = std::list<Order>;
	using BidsMap = std::map<Price, PriceLevelQ, std::greater<Price>>; // best=largest
	using AsksMap = std::map<Price, PriceLevelQ>;                       // best=smallest

	class Book {
		public:
			// Submit a new order (will match then possibly rest)
			void submit(Order o);

			// Cancel by id; returns true if canceled
			bool cancel(OrderId id);

			// Access trades (consumer should drain periodically)
			const std::vector<Trade>& trades() const noexcept { return out_; }
			void clearTrades() { out_.clear(); }

			// Introspection helpers (for tests / monitoring)
			bool   hasBestBid() const noexcept { return !bids_.empty(); }
			bool   hasBestAsk() const noexcept { return !asks_.empty(); }
			Price  bestBid() const { return bids_.begin()->first; }
			Price  bestAsk() const { return asks_.begin()->first; }
			Qty    totalQtyAt(Side s, Price px) const;
			bool   hasOrder(OrderId id) const { return index_.find(id) != index_.end(); }
			size_t numBidLevels() const noexcept { return bids_.size(); }
			size_t numAskLevels() const noexcept { return asks_.size(); }

		private:
			struct Loc { Side side; Price px; PriceLevelQ::iterator it; };

			// Core helpers
			void rest(Order&& o);
			void eraseAt(const Loc& loc);
			Qty  matchIncoming(Order& taker);

			// Books
			BidsMap bids_;
			AsksMap asks_;

			// Fast cancel index
			std::unordered_map<OrderId, Loc> index_;

			// Trade sink (swap with callback/queue in prod)
			std::vector<Trade> out_;
	};

} // namespace ob

