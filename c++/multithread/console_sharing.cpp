#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

std::mutex mtx;
std::condition_variable cv;
std::atomic<int> counter(1);
bool done = false;

void print_non_tens() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [] {
            return done || (counter % 10 != 0);
        });

        if (done) break;

        std::cout << "Thread A: " << counter << std::endl;
        ++counter;

        cv.notify_all();
    }
}

void print_tens() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [] {
            return done || (counter % 10 == 0);
        });

        if (done) break;

        std::cout << "Thread B: " << counter << std::endl;
        ++counter;

        cv.notify_all();
    }
}

int main() {
    int max_value = 40;

    std::thread t1(print_non_tens);
    std::thread t2(print_tens);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        if (counter > max_value) {
            done = true;
            cv.notify_all();
            break;
        }

        cv.wait_for(lock, std::chrono::milliseconds(10));
    }

    t1.join();
    t2.join();

    std::cout << "All done!\n";
    return 0;
}

