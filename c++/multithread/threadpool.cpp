#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <chrono>
#include <future>
#include <optional>
#include <new>


class ThreadPool {
	public:
		explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
			for (size_t i = 0; i < threads; ++i) {
				workers_.emplace_back([this](std::stop_token st) {
						while (!st.stop_requested()) {
						std::move_only_function<void()> task;
						{
						std::unique_lock lock(mu_);
						cv_.wait(lock, [this, &st] {
								return st.stop_requested() || !tasks_.empty();
								});

						if (st.stop_requested() && tasks_.empty())
						return;

						task = std::move(tasks_.front());
						tasks_.pop();
						}
						if (task) {
						try { task(); }
						catch (...) { /* Handle or log exception */ }
						}
						}
						});
			}
		}

		// Delete copy/move to prevent accidental thread ownership issues
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		// Modern Enqueue: Uses move-only tasks to avoid atomic shared_ptr overhead
		template<typename F, typename... Args>
			auto enqueue(F&& f, Args&&... args) {
				using return_type = std::invoke_result_t<F, Args...>;

				// Create a promise to return the result
				auto promise = std::make_shared<std::promise<return_type>>();
				std::future<return_type> res = promise->get_future();

				// Wrap the task in a move-only lambda
				auto task = [func = std::forward<F>(f), ...args = std::forward<Args>(args), promise = std::move(promise)]() mutable {
					try {
						if constexpr (std::is_void_v<return_type>) {
							func(std::forward<Args>(args)...);
							promise->set_value();
						} else {
							promise->set_value(func(std::forward<Args>(args)...));
						}
					} catch (...) {
						promise->set_exception(std::current_exception());
					}
				};

				{
					std::lock_guard lock(mu_);
					tasks_.emplace(std::move(task));
				}
				cv_.notify_one();
				return res;
			}

		~ThreadPool() {
			for (auto& worker : workers_) {
				worker.request_stop();
			}
			cv_.notify_all();
		}

	private:
		// Aligning members to separate cache lines to prevent False Sharing
		alignas(64) std::mutex mu_;
		alignas(64) std::condition_variable cv_;
		alignas(64) std::queue<std::move_only_function<void()>> tasks_;
		std::vector<std::jthread> workers_;
};

class Threadpool {
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;

	std::mutex mu_;
	std::condition_variable cv_;
	bool stop_ = false;

	public:
	Threadpool(size_t threads) {
		for (size_t i = 0; i < threads; ++i) {
			workers_.emplace_back([this, i] {
					for (;;) {
					std::function<void()> task;
					{
					std::unique_lock<std::mutex> lock(mu_);
					cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
					if (stop_ && tasks_.empty()) return;   // drain then exit
					task = std::move(tasks_.front());
					tasks_.pop();
					}
					std::cout << "[Thread " << i << "] Executing task...\n";
					try { task(); } catch (...) { }
					}
					});
		}
	}

	// prevent copying
	Threadpool(const Threadpool&) = delete;
	Threadpool& operator=(const Threadpool&) = delete;

	void enqueue(std::function<void()> task) {
		{
			std::lock_guard<std::mutex> lock(mu_);
			if (stop_) throw std::runtime_error("enqueue on stopped Threadpool");
			tasks_.push(std::move(task));
		}
		cv_.notify_one();
	}

	~Threadpool() {
		{
			std::lock_guard<std::mutex> lock(mu_);
			stop_ = true;
		}
		cv_.notify_all();
		for (auto& worker : workers_) if (worker.joinable()) worker.join();
	}

	template<class F, class... Args>
		auto submit_v1(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
			using R = std::invoke_result_t<F, Args...>;
			auto pkg = std::make_shared<std::packaged_task<R()>> ( std::bind(std::forward<F>(f), std::forward<Args>(args)...));
			std::future<R> fut = pkg->get_future();
			{
				std::unique_lock<std::mutex> lock(mu_);
				if ( stop_ ) throw std::runtime_error("enqueue on stopped Threadpool"); 
				tasks_.emplace([pkg]{ (*pkg)(); });
			}
			cv_.notify_one();
			return fut;
		}

};



class PThreadPool {
	struct Task {
		int priority;
		uint64_t seq;
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
	explicit PThreadPool(size_t threads = std::thread::hardware_concurrency()) {
		for (size_t i = 0; i < threads; ++i) {
			// jthread provides a stop_token and joins automatically on destruction
			workers_.emplace_back([this, i](std::stop_token st) {
					while (true) {
					Task task;
					{
					std::unique_lock<std::mutex> lock(mu_);
					// Wait until a task is available or a stop is requested
					cv_.wait(lock, [this, &st] {
							return st.stop_requested() || !tasks_.empty();
							});

					// Only exit if stop is requested AND no tasks remain in queue
					if (st.stop_requested() && tasks_.empty()) {
					return;
					}

					task = std::move(const_cast<Task&>(tasks_.top()));
					tasks_.pop();
					}

					// Execute the task outside the lock
					try {
						if (task.func) task.func();
					} catch (const std::exception& e) {
						std::cerr << "[Thread " << i << "] Exception: " << e.what() << "\n";
					}
					}
			});
		}
	}

	// Delete copy/move to prevent accidental thread ownership issues
	PThreadPool(const PThreadPool&) = delete;
	PThreadPool& operator=(const PThreadPool&) = delete;

	void enqueue(std::function<void()> func, int priority = 0) {
		{
			std::lock_guard<std::mutex> lock(mu_);
			// Since we use jthread, we check if stop was requested via the worker tokens
			tasks_.push(Task{priority, task_counter_++, std::move(func)});
		}
		cv_.notify_one();
	}

	~PThreadPool() {
		// 1. Request all jthreads to stop
		for (auto& worker : workers_) {
			worker.request_stop();
		}
		// 2. Wake all threads so they can check their stop_tokens and finish tasks
		cv_.notify_all();
		// Note: jthreads automatically join() here in their own destructors.
	}

	private:
	// alignas(64) prevents "False Sharing" by ensuring these members occupy 
	// different cache lines, preventing threads from fighting over the same memory block.
	alignas(64) std::mutex mu_;
	alignas(64) std::condition_variable cv_;
	alignas(64) std::priority_queue<Task> tasks_;

	uint64_t task_counter_ = 0;
	std::vector<std::jthread> workers_;
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

		// returns a value
		auto f1 = pool.enqueue([](int a, int b){ return a + b; }, 10, 32);
		std::cout << "sum : " << f1.get() << std::endl;

		// returns void
		auto f2 = pool.enqueue([]{
				// do some work, no return
				});
		f2.get(); // waits; returns void

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

