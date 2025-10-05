/*
   ================================================================================
   LRUCache (thread-safe, generic, minimal API)
   ================================================================================

   Overview
   --------
   - Eviction policy: Least-Recently-Used (LRU)
   - Operations: 
 * put(Key, Val)                : O(1) average
 * get(const Key&) -> optional  : O(1) average, moves to MRU (mutates)
 * peek(const Key&) -> optional : O(1) average, read-only (no MRU update)
 * print()                      : debug helper (requires streamable Key/Val)
 * size(), capacity()           : O(1)

 Thread-safety
 -------------
 - Internally uses std::shared_mutex:
 * Writers (put, get) take unique_lock (exclusive) because they mutate state
 (get() changes recency by splicing a list node to front).
 * Readers (peek, size, print) take shared_lock (concurrent reads).
 - capacity_ is immutable after construction; capacity() reads it without locking.

 Design
 ------
 - Doubly-linked list (std::list<std::pair<Key,Val>>) keeps recency order:
 front = MRU, back = LRU.
 - unordered_map<Key, list::iterator> gives O(1) hits/updates and ties map nodes
 to list nodes.
 - put(Key, Val) is *by value*: lvalues copy into the parameter; rvalues move.
 Inside, we std::move into list storage to avoid extra copies.

 Requirements
 ------------
 - C++17 (for std::shared_mutex, std::optional).
 - Key must be hashable and equality-comparable (unordered_map requirements).
 - For print(), Key and Val must be streamable (operator<<). If they are not,
 keep print() out of builds or adapt it.

 Build
 -----
 g++ -std=gnu++17 -O2 your_file.cpp -o your_prog
 (Use -pthread if your toolchain needs it for shared_mutex.)

 Gotchas & Tips
 --------------
 - get()/peek() return Val by value. For heavy Val, consider storing Val as
 std::shared_ptr<T> (i.e., Val = std::shared_ptr<T>) to cheap-copy results.
 - If you need heterogeneous lookup (e.g., std::string keys with string_view
 queries), we can extend with transparent hash/equal and additional overloads.

 ================================================================================
 */

#include <list>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <utility>        // std::move
#include <shared_mutex>   // std::shared_mutex, std::shared_lock
#include <mutex>          // std::unique_lock

template<typename Key, typename Val>
class LRUCache {
	private:
		std::size_t cap_;
		std::list<std::pair<Key, Val>> lst_;  // front = MRU, back = LRU
		std::unordered_map<Key, typename std::list<std::pair<Key, Val>>::iterator> map_;
		mutable std::shared_mutex mtx_;       // guards lst_ and map_

	public:
		explicit LRUCache(std::size_t capacity) : cap_(capacity) {
			if (cap_ == 0) throw std::invalid_argument("LRU capacity must be > 0");
		}

		// Single put version (by-value parameters):
		// - lvalue args copy into k/v; rvalue args move into k/v.
		// - We then std::move into the list to avoid extra copies.
		void put(Key k, Val v) {
			std::unique_lock<std::shared_mutex> lock(mtx_);

			auto it = map_.find(k);
			if (it != map_.end()) {
				// Update existing value and move node to MRU.
				it->second->second = std::move(v);
				lst_.splice(lst_.begin(), lst_, it->second);
				return;
			}

			// Evict LRU if at capacity.
			if (map_.size() >= cap_) {
				const Key& oldKey = lst_.back().first;
				map_.erase(oldKey);
				lst_.pop_back();
			}

			// Insert new node as MRU.
			lst_.emplace_front(std::move(k), std::move(v));
			map_[lst_.front().first] = lst_.begin();
		}

		// get: returns value if present, moves the node to MRU (mutates => unique lock).
		std::optional<Val> get(const Key& k) {
			std::unique_lock<std::shared_mutex> lock(mtx_);

			auto it = map_.find(k);
			if (it == map_.end()) return std::nullopt;

			// Touch => move to MRU in O(1)
			lst_.splice(lst_.begin(), lst_, it->second);
			return it->second->second; // copies Val; use shared_ptr Val for cheap copies
		}

		// peek: read-only lookup that DOES NOT change recency (shared lock).
		std::optional<Val> peek(const Key& k) const {
			std::shared_lock<std::shared_mutex> lock(mtx_);

			auto it = map_.find(k);
			if (it == map_.end()) return std::nullopt;

			return it->second->second; // no splice -> no mutation
		}

		// Debug helper (requires streamable Key and Val)
		void print() const {
			std::shared_lock<std::shared_mutex> lock(mtx_);
			std::cout << "Cache [MRU -> LRU]: ";
			for (auto it = lst_.begin(); it != lst_.end(); ++it) {
				std::cout << "(" << it->first << ":" << it->second << ") ";
			}
			std::cout << "\n";
		}

		std::size_t size() const {
			std::shared_lock<std::shared_mutex> lock(mtx_);
			return map_.size();
		}

		std::size_t capacity() const {
			// cap_ is immutable after construction; no lock needed.
			return cap_;
		}
};

int main() {
	LRUCache<int, std::string> lru(3);

	lru.put(1, "one");
	lru.put(2, "two");
	lru.put(3, "three");
	lru.print(); // (3:three) (2:two) (1:one)

	// peek does not change order
	auto p2 = lru.peek(2);
	std::cout << "peek(2): " << (p2 ? *p2 : "<miss>") << "\n";
	lru.print(); // still (3:three) (2:two) (1:one)

	// get changes order (2 becomes MRU)
	auto g2 = lru.get(2);
	std::cout << "get(2): " << (g2 ? *g2 : "<miss>") << "\n";
	lru.print(); // (2:two) (3:three) (1:one)

	// insert new -> evicts LRU (key 1)
	lru.put(4, "four");
	lru.print(); // (4:four) (2:two) (3:three)

	std::cout << "Size: " << lru.size() << " / Capacity: " << lru.capacity() << "\n";
}

