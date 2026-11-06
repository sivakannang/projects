/*
   g++ -std=c++20 -O2 -Wall -Wextra -pthread producer_consumer.cpp -o producer_consumer

   ================================================================================
PROBLEM: Bounded Producer–Consumer

Statement:
Implement a bounded queue with producers and consumers.
The producer must block when the buffer is full; the consumer must block when the buffer is empty.
Cleanly signal completion when the producer finishes.

This file solves it in three ways, plus a semaphore handoff between two threads :
- cv_spsc   : one condition variable and a done flag
- sem_spsc  : two semaphores and a sentinel
- sem_mpmc  : same, with many producers and consumers

Key Ideas with semaphores:
- Two counting semaphores:
slots : counts FREE slots in the buffer (initial = capacity)
items : counts FILLED slots in the buffer (initial = 0)
- A mutex protects the queue itself during push/pop.
- Use a sentinel value to tell the consumer to stop. The sentinel must be a value which real data can never have.

With a single condition variable, producers and consumers wait on the same cv, so with many of them notify_one can wake the
wrong kind, and everyone sleeps. Use two condition variables (not_full, not_empty), or notify_all.

Complexity:
- Each push/pop is O(1).
- No spurious wakeups (semaphores block precisely).
================================================================================



|   Use Case     |  Semaphore                                                         | Condition Variable                                                 |
| ---------------| -------------------------------------------------------------------| -------------------------------------------------------------------|
| What it models | A counter of available resources (permits).                        | A signal to wait until a condition becomes true.                   |
| Best for       | Bounded buffers, rate limiting, connection pools, pipeline handoffs| Complex wait conditions: pause/resume, shutdown, wait until X && !Y|
| How it works   | `acquire()` → take 1 slot, `release()` → add 1 slot.               | `wait(lock, predicate)` → sleep until predicate true.              |
| Wakeups        | Exact (1 release = 1 wake). No spurious wakeups.                   | May wake spuriously; must always recheck predicate.                |
| Shutdown       | Use a sentinel/poison-pill message.                                | Use a `done` flag checked in predicate.                            |
| Mental model   | “Tickets to a resource.”                                           | “Sleep until state changes.”                                       |

✅ Rule of thumb:

- If it’s counting things (slots, items, permits) → use a semaphore.

- If it’s waiting on a complex state condition → use a condition variable.
*/

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <semaphore>    // C++20
#include <syncstream>
#include <chrono>
#include <condition_variable>
using namespace std::chrono_literals;

// ------------------ Single Producer, Single Consumer, condition variable ------------------
namespace cv_spsc {

	std::queue<int> buffer;
	constexpr std::size_t MAX_BUFFER_SIZE = 5;

	std::mutex mtx;
	std::condition_variable cv;
	bool done = false;

	void producer(int items) {
		for (int i = 1; i <= items; ++i) {
			{
				std::unique_lock<std::mutex> lock(mtx);
				cv.wait(lock, [] { return buffer.size() < MAX_BUFFER_SIZE; });
				buffer.push(i);
			}
			cv.notify_one(); // wake a consumer
			std::cout << "Produced: " << i << '\n';
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

}


// ------------------ Single Producer, Single Consumer, semaphores ------------------
namespace sem_spsc {
	static constexpr int CAPACITY = 5;     // bounded buffer
	static constexpr int SENTINEL = -1;    // poison pill

	std::queue<int> q;
	std::mutex q_mtx;
	std::counting_semaphore<CAPACITY> slots(CAPACITY); // free slots
	std::counting_semaphore<CAPACITY> items(0);        // filled items

	void producer(int count) {
		for (int i = 1; i <= count; ++i) {
			slots.acquire();
			{
				std::lock_guard lk(q_mtx);
				q.push(i);
			}
			items.release();
			std::cout << "[P] produced " << i << '\n';
			std::this_thread::sleep_for(100ms);
		}
		// signal completion
		slots.acquire();
		{ std::lock_guard lk(q_mtx); q.push(SENTINEL); }
		items.release();
	}

	void consumer() {
		for (;;) {
			items.acquire();
			int v;
			{
				std::lock_guard lk(q_mtx);
				v = q.front(); q.pop();
			}
			slots.release();
			if (v == SENTINEL) break;
			std::cout << "[C] consumed " << v << '\n';
			std::this_thread::sleep_for(200ms);
		}
	}
}

// ------------------ Multi-Producer, Multi-Consumer, semaphores ------------------
namespace sem_mpmc {
	static constexpr int CAPACITY = 8;
	static constexpr int SENTINEL = -1;

	std::queue<int> q;
	std::mutex q_mtx;
	std::counting_semaphore<CAPACITY> slots(CAPACITY);
	std::counting_semaphore<CAPACITY> items(0);

	void producer(int id, int start, int n) {
		for (int i = 0; i < n; ++i) {
			slots.acquire();
			{
				std::lock_guard lk(q_mtx);
				q.push(start + i);
			}
			items.release();
			{ std::osyncstream(std::cout) << "[P" << id << "] " << start + i << '\n'; }
			std::this_thread::sleep_for(30ms);
		}
	}

	void consumer(int id) {
		for (;;) {
			items.acquire();
			int v;
			{
				std::lock_guard lk(q_mtx);
				v = q.front(); q.pop();
			}
			slots.release();
			if (v == SENTINEL) break;                 // each consumer exits on its own sentinel
			{ std::osyncstream(std::cout) << "[C" << id << "] " << v << '\n'; }
			std::this_thread::sleep_for(60ms);
		}
	}

	// Push N sentinels to stop N consumers (drains all normal items first)
	void stop_consumers(int N) {
		for (int i = 0; i < N; ++i) {
			slots.acquire();
			{ std::lock_guard lk(q_mtx); q.push(SENTINEL); }
			items.release();
		}
	}
}


namespace console_sharing {

	// handoff semaphores
	std::binary_semaphore sem_non_tens(1);  // start enabled
	std::binary_semaphore sem_tens(0);      // start disabled

	// shared state (protected by semaphore handoff)
	int counter = 1;

	void print_non_tens(int max_value) {
		for (;;) {
			sem_non_tens.acquire();

			int cur = counter;                    // safe: only this thread active
			if (cur > max_value) {                // let other thread exit too
				sem_tens.release();
				break;
			}

			{ std::osyncstream(std::cout) << "[A] " << cur << '\n'; }
			++counter;

			// hand off
			if (counter % 10 == 0) sem_tens.release();
			else                   sem_non_tens.release();
		}
	}

	void print_tens(int max_value) {
		for (;;) {
			sem_tens.acquire();

			int cur = counter;                    // safe: only this thread active
			if (cur > max_value) {
				sem_non_tens.release();
				break;
			}

			{ std::osyncstream(std::cout) << "[B] " << cur << '\n'; }
			++counter;

			if (counter % 10 == 0) sem_tens.release();
			else                   sem_non_tens.release();
		}
	}

	inline void run(int max_value) {
		std::thread tA(print_non_tens, max_value);
		std::thread tB(print_tens, max_value);
		tA.join();
		tB.join();
	}

} // namespace console_sharing


int main() {

	{
		using namespace cv_spsc;
		std::cout << "=== SPSC, condition variable ===\n";
		std::thread prod(producer, 10);
		std::thread cons(consumer);
		prod.join();
		cons.join();
		std::cout << "All done.\n";

	}

	{
		using namespace sem_spsc;
		std::cout << "\n=== SPSC, semaphores (1 producer, 1 consumer) ===\n";
		std::thread p(producer, 10);
		std::thread c(consumer);
		p.join(); c.join();
	}

	{
		using namespace sem_mpmc;
		std::cout << "\n=== MPMC, semaphores (2 producers, 3 consumers) ===\n";
		std::thread p1(producer, 1, 100, 10);
		std::thread p2(producer, 2, 200, 10);

		const int N = 3;
		std::thread c1(consumer, 1), c2(consumer, 2), c3(consumer, 3);

		p1.join(); p2.join();
		stop_consumers(N);     // push N sentinels
		c1.join(); c2.join(); c3.join();
	}

	std::cout << "\n=== CONSOLE SHARING print_non_10s, print_10s) ===\n";
	console_sharing::run(40);
}

