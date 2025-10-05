#pragma once
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {

	private:

		std::mutex mtx_;
		std::condition_variable cv_;
		std::queue<std::function<void()>> q_;
		std::vector<std::jthread> workers_;
		bool stopping_{false};


		void worker_loop(std::stop_token st) {
			std::unique_lock<std::mutex> lk(mtx_);
			for (;;) {
				if (q_.empty()) {
					cv_.wait(lk, [&]{ return st.stop_requested() || stopping_ || !q_.empty(); });
					if ((st.stop_requested() || stopping_) && q_.empty()) break;
				}
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
				workers_.emplace_back([this](std::stop_token st){ worker_loop(st); });
			}
		}

		~ThreadPool() { shutdown(); }

		// Enqueue a job for execution.
		void enqueue(std::function<void()> job) {
			{
				std::lock_guard<std::mutex> lk(mtx_);
				if (stopping_) return;               // drop silently after shutdown() requested
				q_.push(std::move(job));
			}
			cv_.notify_one();
		}

		void shutdown() {
			// idempotent
			for (auto& w : workers_) if (w.joinable()) w.request_stop();
			{
				std::lock_guard<std::mutex> lk(mtx_);
				stopping_ = true;
			}
			cv_.notify_all();
			for (auto& w : workers_) if (w.joinable()) w.join();
			workers_.clear();
			// drain any leftover jobs
			std::queue<std::function<void()>> empty; std::swap(q_, empty);
		}

};

