/*
   g++ -std=c++20 -O2 -Wall -Wextra -pthread scheduler.cpp -o scheduler      ( std::jthread needs C++20 )

   Runs tasks at a given time, or repeatedly, on a single timer thread.

   A min-heap of (time, id) gives the next task to run, and a map of id -> task holds the tasks.
   The timer thread sleeps on a condition variable until the next task is due, or until a new earlier task arrives.

   Cancel removes the task from the map, and its heap entry is skipped when it comes up (lazy deletion).
   Removing from the middle of a heap would be O(n).

   Limits of this design :
     - Tasks run on the timer thread, so a slow task delays all the others. ../scheduler_with_tp runs them on a thread pool.
     - Recurring tasks are fixed-delay : the next run is an interval after the last one finishes.
       An every 100ms task taking 50ms runs every 150ms. Fixed-rate would keep to every 100ms.
*/

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

using steadyclock = std::chrono::steady_clock;
using timepoint   = steadyclock::time_point;
using duration    = steadyclock::duration;
using task_id     = unsigned int;
constexpr task_id invalid_task = 0;

struct Task {
	std::function<void()> fn_;
	bool       recurring_{false};
	duration   interval_{};
};

using Node = std::pair<timepoint, task_id>;

struct Comp {
	bool operator()(const Node& a, const Node& b) const noexcept {
		return a.first > b.first; // min-heap
	}
};

class Scheduler {
	private:
		std::mutex mu_;
		std::condition_variable cv_;

		std::atomic<task_id> next_id_{1}; // start at 1; 0 = invalid
		std::priority_queue<Node, std::vector<Node>, Comp> tasks_heap_;
		std::unordered_map<task_id, std::shared_ptr<Task>> tasks_map_;

		bool stopping_{false}; // protected by mu_
		std::once_flag join_once_;

		// Declared last, so the thread starts after all the members above are ready
		std::jthread timer_;

	public:
		Scheduler() : timer_([this] { timer_loop(); }) {}
		~Scheduler() { shutdown(); }

		// Tasks which are waiting don't run. A task which is running finishes first.
		// Can be called many times, and from many threads.
		void shutdown() {
			{
				// stopping_ must change under the lock, else the timer can miss the wakeup
				std::lock_guard<std::mutex> lock(mu_);
				stopping_ = true;
			}
			cv_.notify_all();

			// A task can't join its own thread. The timer loop stops after this task, and the destructor joins.
			if (std::this_thread::get_id() == timer_.get_id()) return;

			std::call_once(join_once_, [this] { timer_.join(); });

			// clean containers under lock, destroy outside lock
			std::priority_queue<Node, std::vector<Node>, Comp> heap_trash;
			std::unordered_map<task_id, std::shared_ptr<Task>> map_trash;
			{
				std::lock_guard<std::mutex> lk(mu_);
				tasks_heap_.swap(heap_trash);
				tasks_map_.swap(map_trash);
			}
		}

		// All of these return invalid_task if the scheduler has stopped
		template<class Fn>
			task_id schedule_at(timepoint tp, Fn fn) {
				return task_add(tp, false, duration::zero(), std::move(fn));   // A time in the past runs as soon as possible
			}

		template<class Fn>
			task_id schedule_after(duration d, Fn fn) {
				if (d < duration::zero()) d = duration::zero();
				return schedule_at(steadyclock::now() + d, std::move(fn));
			}

		template<class Fn>
			task_id schedule_every(duration d, Fn fn) {
				if (d <= duration::zero()) return invalid_task; // avoid zero/negative interval
				return task_add(steadyclock::now() + d, true, d, std::move(fn));
			}

		// Returns true if the task existed, it won't start again. A run in progress finishes.
		// Returns false for an unknown id, a finished task, or a task which is already cancelled.
		bool task_cancel(task_id id) {
			std::lock_guard<std::mutex> lock(mu_);
			return tasks_map_.erase(id) == 1;
		}

	private:
		template<class Fn>
			task_id task_add(timepoint tp, bool recurring, duration interval, Fn fn) {
				auto st = std::make_shared<Task>();
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

		void timer_loop() {
			std::unique_lock<std::mutex> lock(mu_);

			while (!stopping_) {
				// Wait for work or stop
				if (tasks_heap_.empty()) {
					cv_.wait(lock, [&]{ return stopping_ || !tasks_heap_.empty(); });
					continue;
				}

				auto due = tasks_heap_.top().first;
				// Wake early if new earlier task arrives or stop requested
				if (cv_.wait_until(lock, due, [&]{ return stopping_ || tasks_heap_.top().first != due; }))
					continue;

				// Check stopping_ each time. Tasks can keep becoming due, so this loop might never end by itself.
				auto now = steadyclock::now();
				while (!stopping_ && !tasks_heap_.empty() && tasks_heap_.top().first <= now) {
					task_id id = tasks_heap_.top().second;
					tasks_heap_.pop();

					auto it = tasks_map_.find(id);
					if (it == tasks_map_.end()) continue; // cancelled, this is its stale heap entry
					auto task_sp = it->second;

					// Run outside the lock
					lock.unlock();
					try {
						task_sp->fn_();
					} catch (const std::exception& e) {
						std::cerr << "Task " << id << " failed : " << e.what() << '\n';
					} catch (...) {
						std::cerr << "Task " << id << " failed\n";
					}
					lock.lock();

					// Find it again, it may have been cancelled while running
					it = tasks_map_.find(id);
					if (it != tasks_map_.end()) {
						if (task_sp->recurring_) {
							// Fixed-delay : the next run is an interval after this one finished
							tasks_heap_.emplace(steadyclock::now() + task_sp->interval_, id);
						} else {
							tasks_map_.erase(it); // one-shot complete
						}
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
		std::cout << "[" << ms_since_start() << " ms] cancel failed (unknown or finished)\n";
	}

	// keep running a bit more
	std::this_thread::sleep_for(2s);

	sch.task_cancel(slow_id);

	// clean shutdown
	sch.shutdown();
	std::cout << "[" << ms_since_start() << " ms] exiting\n";
	return 0;
}
