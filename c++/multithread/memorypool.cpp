#include <iostream>
#include <vector>
#include <cstddef>
#include <cassert>
#include <mutex>

template <typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool() {
        for (size_t i = 0; i < PoolSize; ++i)
            free_list.push_back(&pool[i]);
    }

    T* allocate() {
        std::lock_guard<std::mutex> lock(mtx);
        if (free_list.empty()) return nullptr;
        T* obj = free_list.back();
        free_list.pop_back();
        return new (obj) T();  // placement new
    }

    void deallocate(T* obj) {
        if (!obj) return;
        obj->~T();  // manually call destructor
        std::lock_guard<std::mutex> lock(mtx);
        free_list.push_back(obj);
    }

private:
    alignas(T) char raw_pool[sizeof(T) * PoolSize];
    T* pool = reinterpret_cast<T*>(raw_pool);
    std::vector<T*> free_list;
    std::mutex mtx;
};


#include <thread>

struct MyObject {
    int x;
    MyObject() : x(0) { std::cout << "Constructed\n"; }
    ~MyObject() { std::cout << "Destroyed\n"; }
};

int main() {
    MemoryPool<MyObject, 10> pool;

    auto worker = [&pool](int id) {
        MyObject* obj = pool.allocate();
        if (obj) {
            obj->x = id;
            std::cout << "Thread " << id << " got obj->x = " << obj->x << "\n";
            pool.deallocate(obj);
        } else {
            std::cout << "Thread " << id << " failed to allocate\n";
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 6; ++i)
        threads.emplace_back(worker, i);

    for (auto& t : threads) t.join();

    return 0;
}

