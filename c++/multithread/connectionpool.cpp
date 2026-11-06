/******************************************* Connection Pool ****************************************
 *
 * g++ -std=c++17 -pthread connectionpool.cpp -o connectionpool
 *
 * Opening a database or network connection is slow, so a pool keeps a few open and lends them out.
 * The custom deleter of shared_ptr returns the connection to the pool, so a caller can't forget it.
 *
 * The queue is in a shared State, and each deleter holds that State. A connection can then be released after
 * the pool is destroyed. A deleter which captures 'this' would use a destroyed pool in that case.
 *
 ****************************************************************************************************/

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct Connection {
    int id;
    explicit Connection(int id) : id(id) {}
};


class ConnectionPool {
public:
    explicit ConnectionPool(std::size_t size) : state(std::make_shared<State>()) {
        for (std::size_t i = 0; i < size; ++i)
            state->pool.push(std::make_unique<Connection>(static_cast<int>(i)));
    }

    // Blocks until a connection is free
    std::shared_ptr<Connection> acquire() {
        std::unique_lock<std::mutex> lock(state->mtx);
        state->cv.wait(lock, [this] { return !state->pool.empty(); });
        return take();
    }

    // Returns nullptr if no connection became free in time
    template <typename Rep, typename Period>
    std::shared_ptr<Connection> try_acquire_for(std::chrono::duration<Rep, Period> timeout) {
        std::unique_lock<std::mutex> lock(state->mtx);
        if (!state->cv.wait_for(lock, timeout, [this] { return !state->pool.empty(); }))
            return nullptr;
        return take();
    }

private:
    struct State {
        std::queue<std::unique_ptr<Connection>> pool;
        std::mutex mtx;
        std::condition_variable cv;
    };

    // The caller holds the lock
    std::shared_ptr<Connection> take() {
        std::unique_ptr<Connection> conn = std::move(state->pool.front());
        state->pool.pop();

        // Wrap connection with custom deleter to return it on release
        return std::shared_ptr<Connection>(conn.release(), [s = state](Connection* c) {
            {
                std::lock_guard<std::mutex> lock(s->mtx);
                s->pool.emplace(c);
            }
            s->cv.notify_one();
        });
    }

    std::shared_ptr<State> state;
};


int main() {
    std::shared_ptr<Connection> outlives_pool;

    {
        ConnectionPool cp(2); // Only 2 connections allowed at a time

        auto worker = [&cp](int tid) {
            auto conn = cp.acquire();  // Acquires connection from pool
            std::cout << ("Thread " + std::to_string(tid) + " : using connection " + std::to_string(conn->id) + "\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            // Auto-returned to pool when shared_ptr goes out of scope
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 5; ++i)
            threads.emplace_back(worker, i);

        for (auto& t : threads)
            t.join();

        auto c1 = cp.acquire();
        outlives_pool = cp.acquire();
        auto c3 = cp.try_acquire_for(std::chrono::milliseconds(50));
        std::cout << "Third connection, when only two exist : " << (c3 ? "acquired" : "timed out") << "\n";
    }

    outlives_pool.reset();     // The pool is gone, but its State is alive until this last release
    std::cout << "Released a connection after the pool was destroyed\n";

    return 0;
}
