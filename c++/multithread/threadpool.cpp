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

class ThreadPool {
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queue_mutex;
	std::condition_variable cv;
	bool stop = false;                    // no atomic needed

	public:
	ThreadPool(size_t threads) {
		for (size_t i = 0; i < threads; ++i) {
			workers.emplace_back([this, i] {
					for (;;) {
					std::function<void()> task;
					{
					std::unique_lock<std::mutex> lock(queue_mutex);
					cv.wait(lock, [this] {
							return stop || !tasks.empty();
							});
					if (stop && tasks.empty()) return;   // drain then exit
					task = std::move(tasks.front());
					tasks.pop();
					}
					std::cout << "[Thread " << i << "] Executing task...\n";
					try {
					task();
					} catch (const std::exception& e) {
					std::cerr << "[Thread " << i << "] task threw: " << e.what() << "\n";
					} catch (...) {
					std::cerr << "[Thread " << i << "] task threw unknown exception\n";
					}
					}
			});
		}
	}

	// prevent copying
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void enqueue(std::function<void()> task) {
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
			tasks.push(std::move(task));
		}
		cv.notify_one();
	}

	~ThreadPool() {
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			stop = true;
		}
		cv.notify_all();
		for (auto& worker : workers) if (worker.joinable()) worker.join();
	}
};


class PThreadPool {
	struct Task {
		int priority;
		uint64_t seq;                    // FIFO within equal priority
		std::function<void()> func;
		bool operator<(const Task& other) const {
			if (priority == other.priority) return seq > other.seq;   // lower seq first
			return priority < other.priority;                          // higher priority first
		}
	};

	std::vector<std::thread> workers;
	std::priority_queue<Task> tasks;

	std::mutex queue_mutex;
	std::condition_variable cv;
	bool stop = false;                   // no atomic needed
	uint64_t task_counter = 0;           // guarded by mutex

	public:
	PThreadPool(size_t threads) {
		for (size_t i = 0; i < threads; ++i) {
			workers.emplace_back([this, i] {
					for (;;) {
					Task task;
					{
					std::unique_lock<std::mutex> lock(queue_mutex);
					cv.wait(lock, [this] {
							return stop || !tasks.empty();
							});
					if (stop && tasks.empty()) return;
					task = std::move(tasks.top());
					tasks.pop();
					}
					std::cout << "[Thread " << i << "] Executing task (priority "
					<< task.priority << ", seq " << task.seq << ")\n";
					try {
					task.func();
					} catch (const std::exception& e) {
					std::cerr << "[Thread " << i << "] task threw: " << e.what() << "\n";
					} catch (...) {
					std::cerr << "[Thread " << i << "] task threw unknown exception\n";
					}
					}
			});
		}
	}

	PThreadPool(const PThreadPool&) = delete;
	PThreadPool& operator=(const PThreadPool&) = delete;

	void enqueue(std::function<void()> func, int priority = 0) {
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			if (stop) throw std::runtime_error("enqueue on stopped PThreadPool");
			tasks.push(Task{priority, task_counter++, std::move(func)});
		}
		cv.notify_one();
	}

	~PThreadPool() {
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			stop = true;
		}
		cv.notify_all();
		for (auto& worker : workers) if (worker.joinable()) worker.join();
	}
};



class RThreadPool {
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;

	std::mutex mtx_;
	std::condition_variable cv_;
	bool stop_ = false;  // guarded by mtx_

	public:
	explicit RThreadPool(std::size_t threads) {
		workers_.reserve(threads);
		for (std::size_t i = 0; i < threads; ++i) {
			workers_.emplace_back([this] {
					for (;;) {
					std::function<void()> job;
					{
					std::unique_lock<std::mutex> lk(mtx_);
					cv_.wait(lk, [this] { return stop_ || !tasks_.empty(); });
					if (stop_ && tasks_.empty()) return;  // drain then exit
					job = std::move(tasks_.front());
					tasks_.pop();
					}
					// Execute outside the lock
					job(); // exceptions are captured by packaged_task; won't escape here
					}
					});
		}
	}

	RThreadPool(const RThreadPool&) = delete;
	RThreadPool& operator=(const RThreadPool&) = delete;

	template <class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
		-> std::future<typename std::invoke_result<F, Args...>::type>
		{
			using R = typename std::invoke_result<F, Args...>::type;

			// Bind callable and args into a task<R()>
			auto task_ptr = std::make_shared<std::packaged_task<R()>>(
					std::bind(std::forward<F>(f), std::forward<Args>(args)...)
					);

			std::future<R> fut = task_ptr->get_future();
			{
				std::lock_guard<std::mutex> lk(mtx_);
				if (stop_) throw std::runtime_error("enqueue on stopped RThreadPool");
				tasks_.emplace([task_ptr]{ (*task_ptr)(); });
			}
			cv_.notify_one();
			return fut;
		}

	~RThreadPool() {
		{
			std::lock_guard<std::mutex> lk(mtx_);
			stop_ = true;
		}
		cv_.notify_all();
		for (auto& t : workers_) if (t.joinable()) t.join();
	}
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

