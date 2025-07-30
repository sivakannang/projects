#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic<int> counter(1);
const int max_value = 40;

void print_non_tens() {
    while (true) {
        int current = counter.load();

        if (current > max_value) break;

        if (current % 10 != 0) {
            std::cout << "Thread A: " << current << std::endl;
            counter.fetch_add(1);
        }
        else {
            std::this_thread::yield();
        }
    }
}

void print_tens() {
    while (true) {
        int current = counter.load();

        if (current > max_value) break;

        if (current % 10 == 0) {
            std::cout << "Thread B: " << current << std::endl;
            counter.fetch_add(1);
        }
        else {
            std::this_thread::yield();
        }
    }
}

int main() {
    std::thread t1(print_non_tens);
    std::thread t2(print_tens);

    t1.join();
    t2.join();

    std::cout << "All done!" << std::endl;
    return 0;
}

