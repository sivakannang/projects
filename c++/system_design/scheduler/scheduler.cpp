#include <iostream>
#include <thread>
#include <queue>
#include <utility>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <chrono>
#include <vector>
#include <condition_variable>
#include <functional>

using steadyclock = std::chrono::steady_clock;
using timepoint   = steadyclock::time_point;
using duration    = steadyclock::duration;
using task_id     = unsigned int;
constexpr task_id invalid_task = 0;

struct Task {
	std::function<void()> fn_;
	timepoint  time_{};
	bool       recurring_{false};
	duration   interval_{};
	bool       cancelled_{false};  // accessed only under mu_
};

using Node = std::pair<timepoint, task_id>;

struct Comp {
	bool operator()(const Node& a, const Node& b) const noexcept {
		return a.first > b.first; // min-heap
	}
};

class Scheduler {
	private:
		std::jthread timer_;
		std::mutex mu_;
		std::condition_variable cv_;

		std::atomic<task_id> next_id_{1}; // start at 1; 0 = invalid
		std::priority_queue<Node, std::vector<Node>, Comp> tasks_heap_;
		std::unordered_map<task_id, std::shared_ptr<Task>> tasks_map_;

		bool stopping_{false}; // protected by mu_

	public:
		Scheduler() {
			timer_ = std::jthread([this](std::stop_token st){ timer_loop(st); });
		}
		~Scheduler() { shutdown(); }

		void shutdown() {
			// request stop and wake
			if (timer_.joinable()) timer_.request_stop();
			{
				std::lock_guard<std::mutex> lock(mu_);
				stopping_ = true;
			}
			cv_.notify_all();
			if (timer_.joinable()) timer_.join();

			// clean containers under lock, destroy outside lock
			std::priority_queue<Node, std::vector<Node>, Comp> heap_trash;
			std::unordered_map<task_id, std::shared_ptr<Task>> map_trash;
			{
				std::lock_guard<std::mutex> lk(mu_);
				tasks_heap_.swap(heap_trash);
				tasks_map_.swap(map_trash);
			}
		}

		template<class Fn>
			task_id schedule_at(timepoint tp, Fn fn) {
				auto now = steadyclock::now();
				if (tp < now) tp = now; // clamp to "run ASAP"
				return task_add(tp, false, duration::zero(), std::move(fn));
			}

		template<class Fn>
			task_id schedule_after(duration d, Fn fn) {
				if (d < duration::zero()) d = duration::zero();
				return schedule_at(steadyclock::now() + d, std::move(fn));
			}

		template<class Fn>
			task_id schedule_every(duration d, Fn fn) {
				if (d <= duration::zero()) return invalid_task; // avoid zero/negative interval
				timepoint tp = steadyclock::now() + d;
				return task_add(tp, true, d, std::move(fn));
			}

		bool task_cancel(task_id id) {
			std::lock_guard<std::mutex> lock(mu_);
			auto it = tasks_map_.find(id);
			if (it == tasks_map_.end()) return false;  // already gone or running
			it->second->cancelled_ = true;             // mark; lazy removal
								   // don't erase here; let the timer thread retire it safely
			cv_.notify_one();
			return true;
		}

	private:
		template<class Fn>
			task_id task_add(timepoint tp, bool recurring, duration interval, Fn fn) {
				auto st = std::make_shared<Task>();
				st->time_     = tp;
				st->recurring_= recurring;
				st->interval_ = interval;
				st->fn_       = std::function<void()>(std::move(fn)); // needs copyable callable

				std::unique_lock<std::mutex> lock(mu_);
				if (stopping_) return invalid_task;
				task_id id = next_id_.fetch_add(1, std::memory_order_relaxed);
				tasks_map_.emplace(id, st);
				tasks_heap_.emplace(tp, id);
				lock.unlock();
				cv_.notify_one(); // in case this became the new earliest
				return id;
			}

		void timer_loop(std::stop_token st) {
			std::unique_lock<std::mutex> lock(mu_);

			for (;;) {
				// Wait for work or stop
				if (tasks_heap_.empty()) {
					cv_.wait(lock, [&]{
							return st.stop_requested() || stopping_ || !tasks_heap_.empty();
							});
					if (st.stop_requested()) break;
					if (stopping_ && tasks_heap_.empty()) break;
				} else {
					auto due = tasks_heap_.top().first;
					// Wake early if new earlier task arrives or stop requested
					cv_.wait_until(lock, due, [&]{
							return st.stop_requested() || stopping_ || tasks_heap_.empty() ||
							tasks_heap_.top().first != due;
							});
					if (st.stop_requested()) break;
					if (stopping_ && tasks_heap_.empty()) break;
					// If woken because earlier arrived, loop will recompute; if time reached, we'll run below.
				}

				auto now = steadyclock::now();
				while (!tasks_heap_.empty() && tasks_heap_.top().first <= now) {
					auto [when, id] = tasks_heap_.top(); tasks_heap_.pop();

					// Lookup current state
					auto it = tasks_map_.find(id);
					if (it == tasks_map_.end()) { continue; } // lazy-deleted/canceled earlier
					auto task_sp = it->second;

					if (task_sp->cancelled_) {
						tasks_map_.erase(it);
						continue;
					}


					// Run outside the lock
					lock.unlock();
					try { task_sp->fn_(); } catch (...) { /* swallow/log */ }
					lock.lock();

					// Re-find; it may have been canceled while running
					auto it2 = tasks_map_.find(id);
					if (it2 == tasks_map_.end()) {
						// someone removed it while we were running
					} else if (task_sp->cancelled_) {
						tasks_map_.erase(it2);
					} else if (task_sp->recurring_) {
						// Fixed-rate: maintain cadence based on last scheduled time
						task_sp->time_ = steadyclock::now() + task_sp->interval_;
						tasks_heap_.emplace(task_sp->time_, id);
					} else {
						tasks_map_.erase(it2); // one-shot complete
					}

					// refresh 'now' so we can drain more due tasks in this pass
					now = steadyclock::now();
				}
			}
		}
};


using namespace std::chrono_literals;

// helpers to print relative timestamps
static const auto g_start = std::chrono::steady_clock::now();
static inline long long ms_since_start() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - g_start).count();
}

int main() {
	Scheduler sch; // starts timer thread

	// 1) one-shot after 500ms
	sch.schedule_after(500ms, []{
			std::cout << "[" << ms_since_start() << " ms] one-shot: after 500ms\n";
			});

	// 2) one-shot at absolute time (now + 800ms)
	auto tp = std::chrono::steady_clock::now() + 800ms;
	sch.schedule_at(tp, []{
			std::cout << "[" << ms_since_start() << " ms] one-shot: at now+800ms\n";
			});

	// 3) recurring every 400ms (quick job)
	task_id fast_id = sch.schedule_every(400ms, []{
			std::cout << "[" << ms_since_start() << " ms] every 400ms (fast)\n";
			});

	// 4) recurring every 700ms but simulate slow work (1s)
	task_id slow_id = sch.schedule_every(700ms, []{
			std::this_thread::sleep_for(1s); // long task blocks the timer thread
			std::cout << "[" << ms_since_start() << " ms] every 700ms (slow task ~1s)\n";
			});

	// let it run for a while
	std::this_thread::sleep_for(3s);

	// cancel the fast recurring task
	if (sch.task_cancel(fast_id)) {
		std::cout << "[" << ms_since_start() << " ms] cancelled 400ms recurring task\n";
	} else {
		std::cout << "[" << ms_since_start() << " ms] cancel failed (already running/finished)\n";
	}

	// keep running a bit more
	std::this_thread::sleep_for(2s);

	// clean shutdown
	sch.shutdown();
	std::cout << "[" << ms_since_start() << " ms] exiting\n";
	return 0;
}


