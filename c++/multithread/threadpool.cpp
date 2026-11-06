/******************************************* Thread Pool ********************************************
 *
 * g++ -std=c++23 -O2 -pthread threadpool.cpp -o threadpool      ( std::move_only_function needs C++23 )
 *
 * A thread pool keeps worker threads running, and they take tasks from a shared queue.
 * It avoids the cost of creating a thread per task, and limits how many run at once.
 *
 * ThreadPool         : Returns a future, tasks can be move only (C++23)
 * BasicThreadPool    : std::function queue, the usual interview answer (C++11 style)
 * PriorityThreadPool : Higher priority first, first in first out within the same priority
 *
 * Shutdown, the part which is easy to get wrong :
 *
 *   The stop flag must change while holding the mutex, even if the flag is atomic.
 *     Without the lock, this order hangs forever :
 *       Worker  : holds the lock, the wait predicate returns false, and it is about to sleep
 *       Destroy : sets stop, calls notify_all, nobody is sleeping yet, so the wakeup is lost
 *       Worker  : sleeps, and the join in the destructor never returns
 *     With the lock, the destructor can't set stop inside that window.
 *
 *   A jthread's request_stop() has the same problem with a plain condition_variable.
 *     Use condition_variable_any::wait(lock, stop_token, predicate), or a flag under the lock as done here.
 *
 *   Exit only when stop is set AND the queue is empty, otherwise queued tasks are dropped,
 *   and their futures throw broken_promise.
 *
 *   A task's exception must not escape the worker, as that terminates the whole program.
 *
 * False sharing : the mutex, condition variable and queue are used together under one lock, so placing them on
 * different cache lines doesn't help. Align data which different threads update at the same time without a shared lock.
 *
 ****************************************************************************************************/

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>


// hardware_concurrency() is allowed to return 0, and a pool with no workers would never run anything
inline std::size_t default_thread_count() {
	return std::max(1u, std::thread::hardware_concurrency());
}


class ThreadPool {
	public:
		explicit ThreadPool(std::size_t threads = default_thread_count()) {
			for (std::size_t i = 0; i < threads; ++i) {
				workers_.emplace_back([this] {
					for (;;) {
						std::move_only_function<void()> task;
						{
							std::unique_lock lock(mu_);
							cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

							if (stop_ && tasks_.empty())   // drain then exit
								return;

							task = std::move(tasks_.front());
							tasks_.pop();
						}
						task();   // Can't throw, enqueue stores any exception in the promise
					}
				});
			}
		}

		// Delete copy/move to prevent accidental thread ownership issues
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		// Arguments are copied or moved into the task, like std::thread. Use std::ref to pass a reference.
		template<typename F, typename... Args>
			auto enqueue(F&& f, Args&&... args) {
				using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

				std::promise<return_type> promise;
				std::future<return_type> res = promise.get_future();

				// move_only_function allows a move only lambda, so the promise needs no shared_ptr
				auto task = [func = std::forward<F>(f), ...args = std::forward<Args>(args), promise = std::move(promise)]() mutable {
					try {
						if constexpr (std::is_void_v<return_type>) {
							std::invoke(std::move(func), std::move(args)...);
							promise.set_value();
						} else {
							promise.set_value(std::invoke(std::move(func), std::move(args)...));
						}
					} catch (...) {
						promise.set_exception(std::current_exception());
					}
				};

				{
					std::lock_guard lock(mu_);
					if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
					tasks_.emplace(std::move(task));
				}
				cv_.notify_one();
				return res;
			}

		~ThreadPool() {
			{
				std::lock_guard lock(mu_);
				stop_ = true;
			}
			cv_.notify_all();
			// workers_ is the last member, so the jthreads join here, before the members they use are destroyed
		}

	private:
		std::mutex mu_;
		std::condition_variable cv_;
		std::queue<std::move_only_function<void()>> tasks_;
		bool stop_ = false;
		std::vector<std::jthread> workers_;
};


class BasicThreadPool {
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;

	std::mutex mu_;
	std::condition_variable cv_;
	bool stop_ = false;

	public:
	explicit BasicThreadPool(std::size_t threads = default_thread_count()) {
		for (std::size_t i = 0; i < threads; ++i) {
			workers_.emplace_back([this] {
				for (;;) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(mu_);
						cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
						if (stop_ && tasks_.empty()) return;   // drain then exit
						task = std::move(tasks_.front());
						tasks_.pop();
					}
					try { task(); }
					catch (const std::exception& e) { std::cerr << "Task failed : " << e.what() << "\n"; }
					catch (...) { std::cerr << "Task failed\n"; }
				}
			});
		}
	}

	// prevent copying
	BasicThreadPool(const BasicThreadPool&) = delete;
	BasicThreadPool& operator=(const BasicThreadPool&) = delete;

	void enqueue(std::function<void()> task) {
		{
			std::lock_guard<std::mutex> lock(mu_);
			if (stop_) throw std::runtime_error("enqueue on stopped BasicThreadPool");
			tasks_.push(std::move(task));
		}
		cv_.notify_one();
	}

	~BasicThreadPool() {
		{
			std::lock_guard<std::mutex> lock(mu_);
			stop_ = true;
		}
		cv_.notify_all();
		for (auto& worker : workers_) worker.join();
	}

	// std::function must be copyable, so the packaged_task, which is move only, is held by a shared_ptr
	template<class F, class... Args>
		auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
			using R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
			auto pkg = std::make_shared<std::packaged_task<R()>>(
				[func = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable -> R {
					return std::invoke(std::move(func), std::move(args)...);
				});
			std::future<R> fut = pkg->get_future();
			enqueue([pkg]{ (*pkg)(); });
			return fut;
		}

};


class PriorityThreadPool {
	struct Task {
		int priority = 0;
		std::uint64_t seq = 0;
		std::function<void()> func;

		// Priority Queue is a Max-Heap:
		// We want HIGHER priority value first, and LOWER seq value for equal priority.
		bool operator<(const Task& other) const {
			if (priority != other.priority) {
				return priority < other.priority;
			}
			return seq > other.seq;
		}
	};

	public:
	explicit PriorityThreadPool(std::size_t threads = default_thread_count()) {
		for (std::size_t i = 0; i < threads; ++i) {
			workers_.emplace_back([this, i] {
				for (;;) {
					Task task;
					{
						std::unique_lock<std::mutex> lock(mu_);
						cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

						// Only exit if stop is requested AND no tasks remain in queue
						if (stop_ && tasks_.empty()) {
							return;
						}

						// top() is const. Moving out is safe, as the order uses only priority and seq, which stay valid for pop().
						task = std::move(const_cast<Task&>(tasks_.top()));
						tasks_.pop();
					}

					// Execute the task outside the lock
					try {
						if (task.func) task.func();
					} catch (const std::exception& e) {
						std::cerr << "[Thread " << i << "] Exception: " << e.what() << "\n";
					} catch (...) {
						std::cerr << "[Thread " << i << "] Unknown exception\n";
					}
				}
			});
		}
	}

	// Delete copy/move to prevent accidental thread ownership issues
	PriorityThreadPool(const PriorityThreadPool&) = delete;
	PriorityThreadPool& operator=(const PriorityThreadPool&) = delete;

	void enqueue(std::function<void()> func, int priority = 0) {
		{
			std::lock_guard<std::mutex> lock(mu_);
			if (stop_) throw std::runtime_error("enqueue on stopped PriorityThreadPool");
			tasks_.push(Task{priority, task_counter_++, std::move(func)});
		}
		cv_.notify_one();
	}

	~PriorityThreadPool() {
		{
			std::lock_guard<std::mutex> lock(mu_);
			stop_ = true;
		}
		cv_.notify_all();
		// workers_ is the last member, so the jthreads join here
	}

	private:
	std::mutex mu_;
	std::condition_variable cv_;
	std::priority_queue<Task> tasks_;
	bool stop_ = false;

	std::uint64_t task_counter_ = 0;
	std::vector<std::jthread> workers_;
};


struct Adder {
	int base;
	int add(int x) const { return base + x; }
};


void test_thread_pool() {
	std::cout << "=== ThreadPool ===\n";

	ThreadPool pool(3);  // 3 worker threads

	std::vector<std::future<void>> futures;
	for (int i = 1; i <= 5; ++i) {
		futures.push_back(pool.enqueue([i] {
			std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
			std::cout << ("[Task " + std::to_string(i) + "] Completed after " + std::to_string(100 * i) + " ms\n");
		}));
	}
	for (auto& f : futures)
		f.get();
	std::cout << "[Main] All tasks are completed.\n";

	// returns a value
	auto f1 = pool.enqueue([](int a, int b){ return a + b; }, 10, 32);
	std::cout << "sum : " << f1.get() << std::endl;

	// returns void
	auto f2 = pool.enqueue([]{
		// do some work, no return
	});
	f2.get(); // waits; returns void

	// member function, and a move only argument
	Adder adder{5};
	std::cout << "member function : " << pool.enqueue(&Adder::add, &adder, 1).get() << std::endl;
	std::cout << "move only argument : " << pool.enqueue([](std::unique_ptr<int> p) { return *p; }, std::make_unique<int>(7)).get() << std::endl;

	// An exception reaches the caller through the future
	auto f3 = pool.enqueue([]() -> int { throw std::runtime_error("task failed"); });
	try {
		f3.get();
	} catch (const std::exception& e) {
		std::cout << "exception from task : " << e.what() << std::endl;
	}
}


void test_basic_thread_pool() {
	std::cout << "\n=== BasicThreadPool ===\n";

	BasicThreadPool pool(2);
	auto f1 = pool.submit([](std::unique_ptr<int> p) { return *p * 2; }, std::make_unique<int>(21));
	std::cout << "submit : " << f1.get() << std::endl;

	pool.enqueue([] { throw 1; });   // Logged by the worker, and the pool keeps running

	auto f2 = pool.submit([] { return 1; });
	int result = f2.get();
	std::cout << "still running : " << result << std::endl;
}


void test_priority_thread_pool() {
	std::cout << "\n=== PriorityThreadPool ===\n";

	PriorityThreadPool pool(1);

	// Keep the only worker busy until all the tasks are queued, so the order depends only on the priority
	std::promise<void> gate;
	std::shared_future<void> ready = gate.get_future().share();
	pool.enqueue([ready] { ready.wait(); }, 100);

	pool.enqueue([] { std::cout << "[Task A1] P=3\n"; }, 3);
	pool.enqueue([] { std::cout << "[Task A2] P=3\n"; }, 3);
	pool.enqueue([] { std::cout << "[Task B1] P=2\n"; }, 2);
	pool.enqueue([] { std::cout << "[Task C1] P=4\n"; }, 4);
	pool.enqueue([] { std::cout << "[Task B2] P=2\n"; }, 2);
	gate.set_value();
}


// The destructor must run all the queued tasks, and must not hang
void test_shutdown() {
	std::cout << "\n=== Shutdown ===\n";

	std::atomic_int ran{0};
	std::vector<std::future<void>> futures;
	{
		ThreadPool pool(1);
		for (int i = 0; i < 5; ++i)
			futures.push_back(pool.enqueue([&ran] { ran.fetch_add(1); }));
	}
	for (auto& f : futures)
		f.get();
	std::cout << "queued 5, ran " << ran << "\n";

	for (int i = 0; i < 5000; ++i) {
		{ ThreadPool pool(2); }
		{ BasicThreadPool pool(2); }
		{ PriorityThreadPool pool(2); }
	}
	std::cout << "created and destroyed each pool 5000 times\n";
}


int main() {
	test_thread_pool();
	test_basic_thread_pool();
	test_priority_thread_pool();
	test_shutdown();
	return 0;
}
