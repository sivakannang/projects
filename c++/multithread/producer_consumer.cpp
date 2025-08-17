#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::queue<int> buffer;
constexpr std::size_t MAX_BUFFER_SIZE = 5;

std::mutex mtx;
std::condition_variable cv;
bool done = false;

void producer(int items) {
	for (int i = 1; i <= items; ++i) {
		int produced = 0;
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [] { return buffer.size() < MAX_BUFFER_SIZE; });
			buffer.push(i);
			produced = i;
		}
		cv.notify_one(); // wake a consumer
		std::cout << "Produced: " << produced << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	{
		std::lock_guard<std::mutex> lk(mtx);
		done = true;
	}
	cv.notify_all(); // wake any waiting consumers so they can exit
}

void consumer() {
	for (;;) {
		int val = 0;
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [] { return !buffer.empty() || done; });
			if (buffer.empty() && done) break;
			val = buffer.front();
			buffer.pop();
		}
		cv.notify_one(); // wake a producer if it's waiting on "not full"
		std::cout << "Consumed: " << val << '\n';
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
}

int main() {
	std::thread prod(producer, 10);
	std::thread cons(consumer);
	prod.join();
	cons.join();
	std::cout << "All done.\n";
}

