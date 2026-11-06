#pragma once
#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool {

	private:

		std::mutex mtx_;
		std::condition_variable cv_;
		std::queue<std::function<void()>> q_;
		bool stopping_{false};
		std::vector<std::jthread> workers_;   // Last member, so the other members are ready when a thread starts


		void worker_loop() {
			std::unique_lock<std::mutex> lk(mtx_);
			for (;;) {
				cv_.wait(lk, [&]{ return stopping_ || !q_.empty(); });
				if (q_.empty()) break;       // stopping, and all the queued jobs are done

				auto job = std::move(q_.front());
				q_.pop();
				lk.unlock();
				try { job(); } catch (...) { /* swallow/log as needed */ }
				lk.lock();
			}
		}

	public:
		explicit ThreadPool(std::size_t workers = std::max(1u, std::thread::hardware_concurrency()))
		{
			workers_.reserve(workers);
			for (std::size_t i = 0; i < workers; ++i) {
				workers_.emplace_back([this]{ worker_loop(); });
			}
		}

		~ThreadPool() { shutdown(); }

		// Returns false if the pool has stopped, then the job won't run
		bool enqueue(std::function<void()> job) {
			{
				std::lock_guard<std::mutex> lk(mtx_);
				if (stopping_) return false;
				q_.push(std::move(job));
			}
			cv_.notify_one();
			return true;
		}

		// Runs all the queued jobs, and then stops. Can be called again, but only from one thread at a time,
		// and not from inside a job, as a thread can't join itself.
		void shutdown() {
			{
				std::lock_guard<std::mutex> lk(mtx_);
				stopping_ = true;
			}
			cv_.notify_all();
			workers_.clear();                    // jthread joins in its destructor
		}

};
