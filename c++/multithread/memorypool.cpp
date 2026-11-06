/******************************************* Memory Pool ********************************************
 *
 * g++ -std=c++20 -O2 -pthread memorypool.cpp -o memorypool
 *
 * A memory pool allocates all the memory up front, and hands out fixed size slots from a free list.
 * allocate and deallocate are O(1), with no system call, so the latency is predictable.
 *
 * MemoryPool            : Single thread, throws when it is full
 * TSMemoryPool          : Many threads, one mutex. Simple and correct, the first choice for sharing a pool.
 * siva::MemoryPool      : Single thread, returns nullptr when it is full
 * siva::TSMemoryPool    : Many threads, lock free
 *
 * Why a lock free free list is hard, and the bug to avoid. Storing the free list as an array and a top index looks
 * simple, but it is NOT safe with a CAS on top alone :
 *   Two threads deallocating both write freeList[top] before their CAS, so one pointer is overwritten and lost, and
 *   the other is added twice. After that the same slot is handed to two threads.
 * siva::TSMemoryPool uses a linked stack (Treiber stack) instead, where only one atomic word decides the new state.
 *
 * ABA problem :
 *   Thread 1 reads head = A, next = B, and is paused.
 *   Other threads pop A, pop B, push A. head is A again, but B is now in use.
 *   Thread 1's CAS still succeeds as head == A, and sets head = B, a slot which is in use.
 *   Fix : head also holds a tag which changes on each update, so the CAS fails when anything happened in between.
 *
 * Limits of all these pools :
 *   A double free is not detected in general, only when the pool is already full. Finding every case needs a flag per slot.
 *   Lock free doesn't mean fast under contention, as threads retry the CAS and fight for the cache line.
 *   The usual low latency design avoids sharing, with one pool per thread.
 *
 ****************************************************************************************************/

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <set>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

template <typename T, std::size_t N>
class MemoryPool {
	alignas(T) unsigned char buffer[N * sizeof(T)];
	T* freeList[N];
	std::size_t top;
	public:
	MemoryPool() : top(N) {
		for (std::size_t i = 0; i < N; ++i)
			freeList[i] = reinterpret_cast<T*>(buffer + i * sizeof(T));
	}

	// A copy would hold pointers into the original's buffer
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;

	T* allocate() {
		if (top == 0) throw std::bad_alloc();
		return freeList[--top];
	}
	void deallocate(T* p) {
		if (top == N) throw std::logic_error("MemoryPool is full, double free ?");
		freeList[top++] = p;
	}

	template <typename... Args>
		T* create(Args&&... args) {
			T* p = allocate();
			try {
				::new (static_cast<void*>(p)) T{std::forward<Args>(args)...}; // **brace-init**
				return p;
			} catch (...) { deallocate(p); throw; }
		}
	void destroy(T* p) { p->~T(); deallocate(p); }
};

template <typename T, std::size_t N>
class TSMemoryPool {
	public:
		TSMemoryPool() noexcept : top_(N) {
			for (std::size_t i = 0; i < N; ++i)
				freelist_[i] = reinterpret_cast<T*>(buffer_ + i * sizeof(T));
		}

		TSMemoryPool(const TSMemoryPool&)            = delete;
		TSMemoryPool& operator=(const TSMemoryPool&) = delete;

		// Thread-safe: multiple threads may call create()
		template <typename... Args>
			T* create(Args&&... args) {
				T* p;
				{
					// Take a slot from the free list under mutex
					std::lock_guard<std::mutex> lock(m_);
					if (top_ == 0) throw std::bad_alloc();
					p = freelist_[--top_];
				}

				try {
					// Use GLOBAL placement-new explicitly with the leading ::
					//  - Avoids any class-specific operator new overloads.
					//  - Calls the standard: void* operator new(std::size_t, void*) noexcept;
					// Cast to void* explicitly to select the correct overload and
					// to document construction into raw storage.
					::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
					return p;
				} catch (...) {
					// If construction throws, return the slot to the pool (thread-safe)
					std::lock_guard<std::mutex> lock(m_);
					freelist_[top_++] = p;
					throw; // rethrow the SAME exception
				}
			}

		// Thread-safe: multiple threads may call destroy()
		void destroy(T* p) noexcept {
			// Call user destructor OUTSIDE the lock to avoid blocking other threads
			p->~T();

			// Return the slot to the free list under mutex
			std::lock_guard<std::mutex> lock(m_);
			assert(top_ < N && "double free ?");
			if (top_ < N)
				freelist_[top_++] = p;
		}

	private:
		// Raw backing storage for N objects of T.
		// Align to T's alignment so placement-new is always safe.
		alignas(T) unsigned char buffer_[N * sizeof(T)];

		// LIFO free list of slots (pointers into buffer_)
		T* freelist_[N];
		std::size_t top_;

		// Single mutex protects the free list (simple and robust)
		std::mutex m_;
};



namespace siva {

	/**
	 * @brief Single-Threaded Memory Pool for low latency code
	 *
	 * - Returns nullptr when it is full, so the caller doesn't pay for an exception.
	 * - [[unlikely]] keeps the failure path out of the hot path's code layout.
	 */
	template <typename T, std::size_t N>
		class MemoryPool {
			static_assert(alignof(T) <= 64, "Slots are aligned to 64 bytes at most");

			private:
				alignas(64) std::byte buffer_[N * sizeof(T)];
				T* freeList_[N];
				std::size_t top_;

			public:
				MemoryPool() : top_(N) {
					for (std::size_t i = 0; i < N; ++i) {
						freeList_[i] = reinterpret_cast<T*>(&buffer_[i * sizeof(T)]);
					}
				}

				// Deleted copy/move to maintain pointer integrity
				MemoryPool(const MemoryPool&) = delete;
				MemoryPool& operator=(const MemoryPool&) = delete;

				/**
				 * @brief Allocs memory. Returns nullptr if full.
				 */
				[[nodiscard]] T* allocate() noexcept {
					if (top_ == 0) [[unlikely]] {
						return nullptr;
					}
					return freeList_[--top_];
				}

				void deallocate(T* p) noexcept {
					assert(p != nullptr && top_ < N);
					if (p != nullptr && top_ < N) [[likely]] {
						freeList_[top_++] = p;
					}
				}

				/**
				 * @brief Constructs object. Returns nullptr if pool is full.
				 * A try block costs nothing on the path where no exception is thrown.
				 */
				template <typename... Args>
					[[nodiscard]] T* create(Args&&... args) {
						T* p = allocate();

						if (p == nullptr) [[unlikely]] {
							return nullptr;
						}

						try {
							return std::construct_at(p, std::forward<Args>(args)...);
						} catch (...) {
							deallocate(p);    // Don't lose the slot
							throw;
						}
					}

				void destroy(T* p) noexcept {
					if (p != nullptr) [[likely]] {
						std::destroy_at(p); // Call destructor
						deallocate(p);     // Return to pool
					}
				}

				std::size_t available() const noexcept { return top_; }
		};


	/**
	 * @brief Lock-free Memory Pool, a Treiber stack of slot indexes
	 *
	 * head_ packs two values in one atomic word, so a single CAS updates both :
	 *   low 32 bits  : index of the first free slot, N means empty
	 *   high 32 bits : tag, changed on each update to prevent ABA
	 *
	 * The tag can wrap around after 4 billion updates. ABA then needs one thread paused for exactly that many updates.
	 */
	template <typename T, std::size_t N>
		class TSMemoryPool {
			static_assert(alignof(T) <= 64, "Slots are aligned to 64 bytes at most");
			static_assert(N < UINT32_MAX, "The index is stored in 32 bits");
			static_assert(std::atomic<std::uint64_t>::is_always_lock_free);

			private:
				// Own cache lines, so threads updating head_ don't slow the threads using the slots
				alignas(64) std::byte buffer_[N * sizeof(T)];

				// next_[i] is the free slot after slot i. It is atomic, as a thread may read it
				// while it is losing the race for slot i, and the winner's later deallocate writes it.
				alignas(64) std::atomic<std::uint32_t> next_[N];

				alignas(64) std::atomic<std::uint64_t> head_;

				static std::uint32_t index_of(std::uint64_t head) noexcept { return static_cast<std::uint32_t>(head); }
				static std::uint32_t tag_of(std::uint64_t head) noexcept { return static_cast<std::uint32_t>(head >> 32); }
				static std::uint64_t make_head(std::uint32_t index, std::uint32_t tag) noexcept {
					return (static_cast<std::uint64_t>(tag) << 32) | index;
				}

			public:
				TSMemoryPool() noexcept : head_(make_head(0, 0)) {
					for (std::size_t i = 0; i < N; ++i) {
						next_[i].store(static_cast<std::uint32_t>(i + 1), std::memory_order_relaxed);
					}
				}

				// Prohibit Copy/Move
				TSMemoryPool(const TSMemoryPool&) = delete;
				TSMemoryPool& operator=(const TSMemoryPool&) = delete;

				/**
				 * @brief Lock-free allocation. Returns nullptr if full.
				 */
				[[nodiscard]] T* allocate() noexcept {
					// acquire pairs with the release in deallocate, so next_ and the slot's memory are seen correctly
					std::uint64_t head = head_.load(std::memory_order_acquire);
					while (true) {
						std::uint32_t index = index_of(head);
						if (index == N) [[unlikely]] {
							return nullptr;
						}

						// This value may be stale, but then head_ has changed, so the CAS fails and it is not used
						std::uint32_t next = next_[index].load(std::memory_order_relaxed);

						if (head_.compare_exchange_weak(head, make_head(next, tag_of(head) + 1),
									std::memory_order_acquire,
									std::memory_order_acquire)) [[likely]] {
							return reinterpret_cast<T*>(&buffer_[index * sizeof(T)]);
						}
						// head is automatically updated on failure, loop continues
					}
				}

				/**
				 * @brief Lock-free deallocation.
				 */
				void deallocate(T* p) noexcept {
					if (p == nullptr) [[unlikely]] return;

					auto offset = reinterpret_cast<std::byte*>(p) - buffer_;
					assert(offset >= 0 && static_cast<std::size_t>(offset) < N * sizeof(T) && offset % sizeof(T) == 0);
					std::uint32_t index = static_cast<std::uint32_t>(offset / sizeof(T));

					std::uint64_t head = head_.load(std::memory_order_relaxed);
					while (true) {
						// The caller owns this slot, so writing its next_ can't conflict with another deallocate
						next_[index].store(index_of(head), std::memory_order_relaxed);

						// release publishes next_ and the slot's memory to the thread which allocates it next
						if (head_.compare_exchange_weak(head, make_head(index, tag_of(head) + 1),
									std::memory_order_release,
									std::memory_order_relaxed)) [[likely]] {
							return;
						}
					}
				}

				template <typename... Args>
					[[nodiscard]] T* create(Args&&... args) {
						T* p = allocate();
						if (p == nullptr) [[unlikely]] return nullptr;

						try {
							return std::construct_at(p, std::forward<Args>(args)...);
						} catch (...) {
							deallocate(p);    // Don't lose the slot
							throw;
						}
					}

				void destroy(T* p) noexcept {
					if (p != nullptr) [[likely]] {
						std::destroy_at(p);
						deallocate(p);
					}
				}

				// Walks the list, so it is correct only when no other thread is using the pool.
				// A shared counter would add a contended cache line to the hot path.
				std::size_t available() const noexcept {
					std::size_t count = 0;
					for (std::uint32_t i = index_of(head_.load(std::memory_order_acquire)); i != N; i = next_[i].load(std::memory_order_relaxed))
						++count;
					return count;
				}
		};

} // namespace siva

struct Point { int x; int y; };

// Many threads allocate and free. A slot handed to two threads at the same time shows up in its flag.
struct Slot { std::atomic<int> used{0}; };

void test_lock_free_pool() {
	static siva::TSMemoryPool<Slot, 8> pool;

	std::vector<Slot*> slots;
	while (Slot* s = pool.create())
		slots.push_back(s);
	for (Slot* s : slots)
		pool.deallocate(s);    // Keep the Slot objects alive in the free slots

	std::atomic<long> duplicates{0}, ops{0};
	std::atomic<bool> stop{false};
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; ++i) {
		threads.emplace_back([&] {
			std::vector<Slot*> held;
			while (!stop.load(std::memory_order_relaxed)) {
				for (int k = 0; k < 2; ++k) {
					if (Slot* s = pool.allocate()) {
						if (s->used.exchange(1) != 0)
							duplicates.fetch_add(1);
						held.push_back(s);
					}
				}
				for (Slot* s : held) {
					s->used.store(0);
					pool.deallocate(s);
					ops.fetch_add(1, std::memory_order_relaxed);
				}
				held.clear();
			}
		});
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	stop = true;
	for (auto& t : threads)
		t.join();

	std::set<Slot*> distinct;
	std::size_t free_slots = pool.available();
	while (Slot* s = pool.allocate())
		distinct.insert(s);

	std::cout << "Lock free pool : " << ops << " frees by 4 threads, slots given twice : " << duplicates
		<< ", free at end : " << free_slots << ", distinct : " << distinct.size() << " of 8\n";
}

struct Throws { Throws() { throw std::runtime_error("constructor failed"); } };

void test_throwing_constructor() {
	siva::MemoryPool<Throws, 2> pool;
	for (int i = 0; i < 2; ++i) {
		try { (void)pool.create(); }
		catch (const std::runtime_error&) { }
	}
	std::cout << "After throwing constructors, free slots : " << pool.available() << " of 2\n";
}

int main() {
	MemoryPool<Point, 4> pool;

	auto* p1 = pool.create(1,2);
	auto* p2 = pool.create(3,4);

	std::cout << p1->x << "," << p1->y << "  " << p2->x << "," << p2->y << "\n";

	pool.destroy(p2);
	pool.destroy(p1);

	test_throwing_constructor();
	test_lock_free_pool();

	return 0;
}
