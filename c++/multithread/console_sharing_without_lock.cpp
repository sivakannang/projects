#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <syncstream>  // C++20

std::atomic<int> counter{1};
constexpr int max_value = 40;

void print_non_tens() {
	for (;;) {
		int current = counter.load(std::memory_order_relaxed);
		if (current > max_value) break;

		if (current % 10 != 0) {
			std::osyncstream(std::cout) << "Thread A: " << current << '\n';
			counter.fetch_add(1, std::memory_order_relaxed);
		} else {
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
	}
}

void print_tens() {
	for (;;) {
		int current = counter.load(std::memory_order_relaxed);
		if (current > max_value) break;

		if (current % 10 == 0) {
			std::osyncstream(std::cout) << "Thread B: " << current << '\n';
			counter.fetch_add(1, std::memory_order_relaxed);
		} else {
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
	}
}

int main() {
	std::thread t1(print_non_tens), t2(print_tens);
	t1.join(); t2.join();
	std::cout << "All done!\n";
}

