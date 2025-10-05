
#include <iostream>
#include <cstddef>
#include <new>
#include <utility>
#include <mutex>

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

