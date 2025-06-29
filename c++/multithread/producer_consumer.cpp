#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::queue<int> buffer;
const unsigned int MAX_BUFFER_SIZE = 5;

std::mutex mtx;
std::condition_variable cv_producer, cv_consumer;

bool done = false;

void producer(int items) {
    for (int i = 1; i <= items; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_producer.wait(lock, []() {
            return buffer.size() < MAX_BUFFER_SIZE;
        });

        buffer.push(i);
        std::cout << "Produced: " << i << "\n";

        lock.unlock();
        cv_consumer.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Signal done
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
    cv_consumer.notify_all();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_consumer.wait(lock, [] {
            return !buffer.empty() || done;
        });

        if (buffer.empty() && done) break;

        int val = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << val << "\n";

        lock.unlock();
        cv_producer.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}



int main() {
    std::thread prod(producer, 10);
    std::thread cons(consumer);

    prod.join();
    cons.join();

    std::cout << "All done.\n";
    return 0;
}

