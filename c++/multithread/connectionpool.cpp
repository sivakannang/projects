#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <iostream>
#include <thread>

struct Connection {
    int id;
    Connection(int id) : id(id) {}
    void use() const { std::cout << "Using connection " << id << "\n"; }
};


class ConnectionPool {
public:
    ConnectionPool(size_t size) {
        for (size_t i = 0; i < size; ++i)
            pool.push(std::make_shared<Connection>(i));
    }

    std::shared_ptr<Connection> acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !pool.empty(); });

        auto conn = pool.front();
        pool.pop();

        // Wrap connection with custom deleter to return it on release
        return std::shared_ptr<Connection>(conn.get(), [this, conn](Connection*) {
            std::lock_guard<std::mutex> lock(mtx);
            pool.push(conn);
            cv.notify_one();
        });
    }

private:
    std::queue<std::shared_ptr<Connection>> pool;
    std::mutex mtx;
    std::condition_variable cv;
};





int main() {
    ConnectionPool cp(2); // Only 2 connections allowed at a time

    auto worker = [&cp](int tid) {
        auto conn = cp.acquire();  // Acquires connection from pool
        std::cout << "Thread " << tid << ": ";
        conn->use();               // Use connection
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // Auto-returned to pool when shared_ptr goes out of scope
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    return 0;
}

