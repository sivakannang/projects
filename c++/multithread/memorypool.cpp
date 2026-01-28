
#include <iostream>
#include <cstddef>
#include <new>
#include <utility>
#include <mutex>
#include <atomic>
#include <cassert>

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
	T* allocate() {
		if (top == 0) throw std::bad_alloc();
		return freeList[--top];
	}
	void deallocate(T* p) { freeList[top++] = p; }

	template <typename... Args>
		T* create(Args&&... args) {
			T* p = allocate();
			try {
				::new (p) T{std::forward<Args>(args)...}; // **brace-init**
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
			freelist_[top_++] = p;
		}

	private:
		// Raw backing storage for N objects of T.
		// Align to T's alignment so placement-new is always safe.
		alignas(alignof(T)) unsigned char buffer_[N * sizeof(T)];

		// LIFO free list of slots (pointers into buffer_)
		T* freelist_[N];
		std::size_t top_;

		// Single mutex protects the free list (simple and robust)
		std::mutex m_;
};



namespace siva {

	/**
	 * @brief HFT-Optimized Single-Threaded Memory Pool (C++26 Ready)
	 * 
	 * STRATEGY: 
	 * - Returns nullptr on failure (No-Throw guarantee for latency).
	 * - Branch hinting for the "Success" path.
	 */
	template <typename T, std::size_t N>
		class MemoryPool {
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
				 * noexcept ensures the compiler optimizes for zero exceptions.
				 */
				[[nodiscard]] T* allocate() noexcept {
					if (top_ == 0) [[unlikely]] { 
						return nullptr; 
					}
					return freeList_[--top_];
				}

				void deallocate(T* p) noexcept {
					assert(p != nullptr && top_ < N);
					if (p != nullptr) [[likely]] {
						freeList_[top_++] = p;
					}
				}

				/**
				 * @brief Constructs object. Returns nullptr if pool is full.
				 */
				template <typename... Args>
					[[nodiscard]] T* create(Args&&... args) {
						T* p = allocate();

						if (p == nullptr) [[unlikely]] {
							return nullptr;
						}

						// std::construct_at can throw if the Constructor of T throws.
						// In HFT, we usually ensure T's constructor is also noexcept.
						return std::construct_at(p, std::forward<Args>(args)...);
					}

				void destroy(T* p) noexcept {
					if (p != nullptr) [[likely]] {
						std::destroy_at(p); // Call destructor
						deallocate(p);     // Return to pool
					}
				}

				std::size_t available() const noexcept { return top_; }
		};


	template <typename T, std::size_t N>
		class TSMemoryPool {
			private:
				// Separate buffer and metadata by 64 bytes to prevent False Sharing.
				// This ensures threads updating 'top_' don't stall threads reading 'buffer_'.
				alignas(64) std::byte buffer_[N * sizeof(T)];
				alignas(64) T* freeList_[N];

				// Aligned to its own cache line to prevent interference.
				alignas(64) std::atomic<std::size_t> top_;

			public:
				TSMemoryPool() : top_(N) {
					for (std::size_t i = 0; i < N; ++i) {
						freeList_[i] = reinterpret_cast<T*>(&buffer_[i * sizeof(T)]);
					}
				}

				// Prohibit Copy/Move
				TSMemoryPool(const TSMemoryPool&) = delete;
				TSMemoryPool& operator=(const TSMemoryPool&) = delete;

				/**
				 * @brief Lock-free allocation (No-Throw).
				 */
				[[nodiscard]] T* allocate() noexcept {
					std::size_t current_top = top_.load(std::memory_order_relaxed);
					while (true) {
						if (current_top == 0) [[unlikely]] {
							return nullptr; // No-Throw: Deterministic latency for HFT
						}

						// acquire ensures we see the pointer written by deallocate()
						if (top_.compare_exchange_weak(current_top, current_top - 1,
									std::memory_order_acquire, 
									std::memory_order_relaxed)) [[likely]] {
							return freeList_[current_top - 1];
						}
						// current_top is automatically updated on failure, loop continues
					}
				}

				/**
				 * @brief Lock-free deallocation.
				 * Fix: Writes pointer to freeList BEFORE incrementing top_ visibility.
				 */
				void deallocate(T* p) noexcept {
					if (p == nullptr) [[unlikely]] return;

					std::size_t current_top = top_.load(std::memory_order_relaxed);
					while (true) {
						if (current_top >= N) [[unlikely]] {
							// This would be a logic error (double deallocate or wrong pool)
							assert(false && "Pool overflow");
							return;
						}

						// 1. DATA-FIRST WRITE: We prepare the slot while it's still 'private'
						freeList_[current_top] = p;

						// 2. ATOMIC RELEASE: 
						// release ensures the write to freeList_[current_top] is visible 
						// to any thread that performs a successful 'acquire' in allocate().
						if (top_.compare_exchange_weak(current_top, current_top + 1,
									std::memory_order_release, 
									std::memory_order_relaxed)) [[likely]] {
							return;
						}
						// If another thread pushed/popped, current_top is updated and we try again
					}
				}

				template <typename... Args>
					[[nodiscard]] T* create(Args&&... args) {
						T* p = allocate();
						if (p == nullptr) [[unlikely]] return nullptr;

						// noexcept constructors are preferred in HFT
						return std::construct_at(p, std::forward<Args>(args)...);
					}

				void destroy(T* p) noexcept {
					if (p != nullptr) [[likely]] {
						std::destroy_at(p);
						deallocate(p);
					}
				}

				std::size_t available() const noexcept {
					return top_.load(std::memory_order_relaxed);
				}
		};

} // namespace siva

struct Point { int x; int y; };

int main() {
	MemoryPool<Point, 4> pool;

	auto* p1 = pool.create(1,2);
	auto* p2 = pool.create(3,4);

	std::cout << p1->x << "," << p1->y << "  " << p2->x << "," << p2->y << "\n";

	pool.destroy(p2);
	pool.destroy(p1);

	return 0;
}

