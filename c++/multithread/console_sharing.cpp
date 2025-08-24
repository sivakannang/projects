#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;

int  counter   = 1;      // protected by mtx
int  max_value = 40;     // read-only after init

void print_non_tens() {
	std::unique_lock<std::mutex> lock(mtx);
	while (counter <= max_value) {
		cv.wait(lock, [] {
				return (counter > max_value) || (counter % 10 != 0);
				});
		if (counter > max_value) { cv.notify_one(); break; }

		std::cout << "Thread A: " << counter << '\n';
		++counter;
		cv.notify_one();
	}
}

void print_tens() {
	std::unique_lock<std::mutex> lock(mtx);
	while (counter <= max_value) {
		cv.wait(lock, [] {
				return (counter > max_value) || (counter % 10 == 0);
				});
		if (counter > max_value) { cv.notify_one(); break;}

		std::cout << "Thread B: " << counter << '\n';
		++counter;
		cv.notify_one();
	}
}

int main() {
	std::thread t1(print_non_tens);
	std::thread t2(print_tens);

	t1.join();
	t2.join();

	std::cout << "All done!\n";
	return 0;
}

