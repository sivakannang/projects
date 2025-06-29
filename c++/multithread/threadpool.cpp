#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <chrono>

class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> stop = false;

public:
    ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    std::function<void()> task;

                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    cv.wait(lock, [this] {
                        return !tasks.empty() || stop;
                    });

                    if (stop && tasks.empty())
                        return;

                    task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();

                    std::cout << "[Thread " << i << "] Executing task...\n";
                    task();
                }
            });
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        cv.notify_one();
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (auto& worker : workers)
            if (worker.joinable())
                worker.join();
    }
};


class PThreadPool {
    struct Task {
        int priority;
        uint64_t seq; // for FIFO ordering
        std::function<void()> func;

        // Order by priority (high first), then FIFO (low seq first)
        bool operator<(const Task& other) const {
            if (priority == other.priority)
                return seq > other.seq; // lower seq first
            return priority < other.priority; // higher priority first
        }
    };

    std::vector<std::thread> workers;
    std::priority_queue<Task> tasks;

    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> stop = false;
    std::atomic<uint64_t> task_counter = 0; // monotonically increasing seq
public:
    PThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    Task task;

                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this] {
                        return !tasks.empty() || stop;
                    });

                    if (stop && tasks.empty())
                        return;

                    task = std::move(tasks.top());
                    tasks.pop();
                    lock.unlock();

                    std::cout << "[Thread " << i << "] Executing task (priority " 
                              << task.priority << ", seq " << task.seq << ")\n";
                    task.func();
                }
            });
        }
    }

    void enqueue(std::function<void()> func, int priority = 0) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.push(Task{priority, task_counter++, std::move(func)});
        }
        cv.notify_one();
    }

    ~PThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (auto& worker : workers)
            if (worker.joinable())
                worker.join();
    }
};


// === Sample Usage ===
int main() {


    {
    ThreadPool pool(3);  // 3 worker threads

    // Enqueue some sample tasks
    for (int i = 1; i <= 5; ++i) {
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            std::cout << "[Task " << i << "] Completed after " << (100 * i) << " ms\n";
        });
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[Main] All tasks should be completed by now.\n";
    }


    {
    PThreadPool pool(2);
    pool.enqueue([] { std::cout << "[Task A1] P=3\n"; }, 3);
    pool.enqueue([] { std::cout << "[Task A2] P=3\n"; }, 3);
    pool.enqueue([] { std::cout << "[Task B1] P=2\n"; }, 2);
    pool.enqueue([] { std::cout << "[Task C1] P=4\n"; }, 4);
    pool.enqueue([] { std::cout << "[Task B2] P=2\n"; }, 2);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "[Main] Done.\n";
    }


    return 0;
}

