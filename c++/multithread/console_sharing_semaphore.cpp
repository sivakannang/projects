#include <iostream>
#include <thread>
#include <semaphore>
#include <atomic>

std::counting_semaphore<1> sem_non_tens(1); // start at 1 (counter==1 → non-ten)
std::counting_semaphore<1> sem_tens(0);

std::atomic<int> counter{1};
constexpr int max_value = 40;

void print_non_tens() {
	for (;;) {
		sem_non_tens.acquire();
		int cur = counter.load(std::memory_order_relaxed);
		if (cur > max_value) {       // ensure the other thread can exit too
			sem_tens.release();
			break;
		}
		std::cout << "Thread A: " << cur << '\n';
		++counter;
		int next = counter.load(std::memory_order_relaxed);
		(next % 10 == 0 ? sem_tens : sem_non_tens).release();
	}
}

void print_tens() {
	for (;;) {
		sem_tens.acquire();
		int cur = counter.load(std::memory_order_relaxed);
		if (cur > max_value) {       // ensure the other thread can exit too
			sem_non_tens.release();
			break;
		}
		std::cout << "Thread B: " << cur << '\n';
		++counter;
		int next = counter.load(std::memory_order_relaxed);
		(next % 10 == 0 ? sem_tens : sem_non_tens).release();
	}
}

int main() {
	std::thread a(print_non_tens);
	std::thread b(print_tens);
	a.join();
	b.join();
	std::cout << "All done!\n";
}

