#include "order_book.hpp"
#include <algorithm>

namespace ob {

	Qty Book::totalQtyAt(Side s, Price px) const {
		Qty sum = 0;
		if (s == Side::Buy) {
			auto it = bids_.find(px);
			if (it == bids_.end()) return 0;
			for (auto& o : it->second) sum += o.qty;
		} else {
			auto it = asks_.find(px);
			if (it == asks_.end()) return 0;
			for (auto& o : it->second) sum += o.qty;
		}
		return sum;
	}

	void Book::rest(Order&& o) {
		if (o.side == Side::Buy) {
			auto& q = bids_[o.px];
			q.emplace_back(std::move(o));          // FIFO tail
			auto it = std::prev(q.end());
			index_[it->id] = Loc{Side::Buy, it->px, it};
		} else {
			auto& q = asks_[o.px];
			q.emplace_back(std::move(o));
			auto it = std::prev(q.end());
			index_[it->id] = Loc{Side::Sell, it->px, it};
		}
	}

	void Book::eraseAt(const Loc& loc) {
		if (loc.side == Side::Buy) {
			auto itLevel = bids_.find(loc.px);
			if (itLevel == bids_.end()) return;
			auto& q = itLevel->second;
			q.erase(loc.it);
			if (q.empty()) bids_.erase(itLevel);
		} else {
			auto itLevel = asks_.find(loc.px);
			if (itLevel == asks_.end()) return;
			auto& q = itLevel->second;
			q.erase(loc.it);
			if (q.empty()) asks_.erase(itLevel);
		}
	}

	Qty Book::matchIncoming(Order& taker) {
		auto canCross = [&](Side s) -> bool {
			if (s == Side::Buy) {
				if (asks_.empty()) return false;
				return taker.type == Type::Market || taker.px >= asks_.begin()->first;
			} else {
				if (bids_.empty()) return false;
				return taker.type == Type::Market || taker.px <= bids_.begin()->first;
			}
		};

		while (taker.qty > 0 && canCross(taker.side)) {
			if (taker.side == Side::Buy) {
				auto itLevel = asks_.begin();      // best ask
				Price tradePx = itLevel->first;
				auto& q = itLevel->second;

				while (taker.qty > 0 && !q.empty()) {
					Order& maker = q.front();
					Qty fill = std::min(taker.qty, maker.qty);

					out_.push_back(Trade{maker.id, taker.id, tradePx, fill});

					taker.qty -= fill;
					maker.qty -= fill;

					if (maker.qty == 0) {
						index_.erase(maker.id);
						q.pop_front();
					} else {
						break; // partial; maker stays
					}
				}
				if (q.empty()) asks_.erase(itLevel);
			} else { // taker is Sell
				auto itLevel = bids_.begin();      // best bid
				Price tradePx = itLevel->first;
				auto& q = itLevel->second;

				while (taker.qty > 0 && !q.empty()) {
					Order& maker = q.front();
					Qty fill = std::min(taker.qty, maker.qty);

					out_.push_back(Trade{maker.id, taker.id, tradePx, fill});

					taker.qty -= fill;
					maker.qty -= fill;

					if (maker.qty == 0) {
						index_.erase(maker.id);
						q.pop_front();
					} else {
						break;
					}
				}
				if (q.empty()) bids_.erase(itLevel);
			}
		}
		return taker.qty;
	}

	void Book::submit(Order o) {
		// 1) Match as taker
		Qty remaining = matchIncoming(o);

		// 2) Post-trade handling
		if (remaining > 0) {
			// MARKET or IOC should not rest
			if (o.type == Type::Market || o.tif == TIF::IOC) return;

			// Safety: do not rest if it still crosses best opposite at this moment
			if (o.side == Side::Buy && !asks_.empty() && o.px >= asks_.begin()->first) return;
			if (o.side == Side::Sell && !bids_.empty() && o.px <= bids_.begin()->first) return;

			o.qty = remaining;
			rest(std::move(o));
		}
	}

	bool Book::cancel(OrderId id) {
		auto it = index_.find(id);
		if (it == index_.end()) return false;
		eraseAt(it->second);
		index_.erase(it);
		return true;
	}

} // namespace ob

