#include <iostream>
#include <vector>
#include <mutex>

template <typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool() {
        for (size_t i = 0; i < PoolSize; ++i) {
            void* ptr = &raw_pool[i * sizeof(T)];
            free_list.push_back(reinterpret_cast<T*>(ptr));
        }
    }

    template<typename... Args>
    T* allocate(Args&&... args) {
        std::lock_guard<std::mutex> lock(mtx);

        if (free_list.empty()) {
            std::cout << "Memory pool exhausted\n";
            return nullptr;
        }

        T* obj = free_list.back();
        free_list.pop_back();

        // placement new
        return new (obj) T(std::forward<Args>(args)...);
    }

    void deallocate(T* obj) {
        if (!obj) return;

        obj->~T(); // manually call destructor

        std::lock_guard<std::mutex> lock(mtx);
        free_list.push_back(obj);
    }

private:
    alignas(T) char raw_pool[sizeof(T) * PoolSize];
    std::vector<T*> free_list;
    std::mutex mtx;
};



struct Point {
    int x, y;

    Point(int a, int b) : x(a), y(b) {
        std::cout << "Point constructed: " << x << "," << y << "\n";
    }

    ~Point() {
        std::cout << "Point destroyed: " << x << "," << y << "\n";
    }
};

int main() {
    MemoryPool<Point, 3> pool;

    Point* p1 = pool.allocate(1, 2);
    Point* p2 = pool.allocate(3, 4);
    Point* p3 = pool.allocate(5, 6);
    Point* p4 = pool.allocate(7, 8); // pool exhausted

    pool.deallocate(p2);
    pool.deallocate(p1);

    Point* p5 = pool.allocate(9, 10);

    pool.deallocate(p3);
    pool.deallocate(p5);
}

