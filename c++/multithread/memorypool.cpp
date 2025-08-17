
#include <iostream>
#include <cstddef>
#include <new>
#include <utility>

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

