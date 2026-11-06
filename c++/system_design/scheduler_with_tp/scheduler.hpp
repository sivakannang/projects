#pragma once
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "thread_pool.hpp"

// Runs tasks at a given time, or repeatedly, on a thread pool. So a slow task doesn't delay the others.
//
// The timer thread only decides when a task is due, and hands it to the pool.
// A recurring task is scheduled again when its run finishes, so one task never overlaps itself.
//
// Lifetime : a job in the pool uses this Scheduler when it finishes, so shutdown() waits for all the running jobs.
//            Without that wait, the Scheduler could be destroyed while a job is still using it.
//            The pool must outlive the Scheduler.
class Scheduler {
public:
    using clock      = std::chrono::steady_clock;   // monotonic
    using time_point = clock::time_point;
    using duration   = clock::duration;

    static constexpr uint64_t invalid_task = 0;

    explicit Scheduler(ThreadPool& pool) : pool_(pool), timer_([this]{ timer_loop(); }) {}

    ~Scheduler() { shutdown(); }

    // All the schedule functions return invalid_task if the scheduler has stopped

    // One-shot at absolute time. A time in the past runs as soon as possible.
    template <class Fn>
    uint64_t schedule_at(time_point tp, Fn fn) {
        return add_task(tp, duration::zero(), /*recurring*/false, /*fixed_rate*/true, std::move(fn));
    }

    // One-shot after delay
    template <class Fn>
    uint64_t schedule_after(duration d, Fn fn) {
        return schedule_at(clock::now() + d, std::move(fn));
    }

    // Recurring, the interval must be positive, else it returns invalid_task
    // fixed_rate=true  : keep to the original cadence. Runs missed by a slow task are skipped, not run later to catch up.
    // fixed_rate=false : fixed delay, the next run is an interval after this one finishes
    template <class Fn>
    uint64_t schedule_every(duration interval, Fn fn, bool fixed_rate = true) {
        if (interval <= duration::zero()) return invalid_task;
        return add_task(clock::now() + interval, interval, /*recurring*/true, fixed_rate, std::move(fn));
    }

    // Returns true if the task existed, it won't start again. A run in progress finishes.
    // Returns false for an unknown id, a finished task, or a task which is already cancelled.
    bool cancel(uint64_t id) {
        std::lock_guard<std::mutex> lk(mu_);
        return tasks_.erase(id) == 1;       // Its heap entry is skipped when it comes up
    }

    // Tasks which are waiting don't run. Waits for the jobs which were handed to the pool.
    // Called from inside a task it only requests the stop, as that job can't wait for itself. The destructor waits.
    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(mu_);
            stopping_ = true;
        }
        cv_.notify_all();

        if (current_scheduler() == this) return;

        std::call_once(join_once_, [this]{ timer_.join(); });

        std::unique_lock<std::mutex> lk(mu_);
        idle_cv_.wait(lk, [this]{ return in_flight_ == 0; });
        heap_ = {};
        tasks_.clear();
    }

private:
    struct TaskState {
        std::function<void()> fn;
        duration              interval{};
        bool                  recurring {false};
        bool                  fixed_rate{true};
    };

    using Node = std::pair<time_point, uint64_t>; // (when, id)
    struct MinTime {
        bool operator()(const Node& a, const Node& b) const noexcept {
            return a.first > b.first; // min-heap
        }
    };

    // The scheduler whose job is running on this thread, nullptr if none
    static const Scheduler*& current_scheduler() {
        thread_local const Scheduler* current = nullptr;
        return current;
    }

    template <class Fn>
    uint64_t add_task(time_point tp, duration iv, bool recurring, bool fixed_rate, Fn fn) {
        auto st = std::make_shared<TaskState>();
        st->fn         = std::function<void()>(std::move(fn)); // requires copyable callable
        st->interval   = iv;
        st->recurring  = recurring;
        st->fixed_rate = fixed_rate;

        uint64_t id;
        {
            std::lock_guard<std::mutex> lk(mu_);
            if (stopping_) return invalid_task;
            id = next_id_++;
            tasks_.emplace(id, st);
            heap_.emplace(tp, id);
        }
        cv_.notify_one();
        return id;
    }

    void timer_loop() {
        std::unique_lock<std::mutex> lk(mu_);
        while (!stopping_) {
            // stop/idle logic
            if (heap_.empty()) {
                cv_.wait(lk, [&]{ return stopping_ || !heap_.empty(); });
                continue;
            }

            auto next_tp = heap_.top().first;
            if (cv_.wait_until(lk, next_tp, [&]{ return stopping_ || heap_.top().first != next_tp; }))
                continue; // stopping, or an earlier task arrived

            // run due tasks
            auto now = clock::now();
            while (!stopping_ && !heap_.empty() && heap_.top().first <= now) {
                auto [when, id] = heap_.top();
                heap_.pop();

                auto it = tasks_.find(id);
                if (it == tasks_.end()) continue;   // cancelled, this is its stale heap entry

                // enqueue only adds to the pool's queue, so holding mu_ here doesn't wait for the work.
                // The jobs take mu_ without holding the pool's lock, so the two locks can't deadlock.
                ++in_flight_;
                if (!pool_.enqueue([this, id = id, when = when, task = it->second] { run_job(id, when, *task); })) {
                    // The pool has stopped, so this task can't run
                    --in_flight_;
                    tasks_.erase(id);
                }
            }
        }
    }

    // Runs on a pool thread
    void run_job(uint64_t id, time_point scheduled, const TaskState& task) {
        current_scheduler() = this;
        try { task.fn(); } catch (...) { /* log/swallow */ }
        current_scheduler() = nullptr;

        std::lock_guard<std::mutex> lk(mu_);

        auto it = tasks_.find(id);
        if (it != tasks_.end()) {               // Else it was cancelled during the run
            if (task.recurring && !stopping_) {
                heap_.emplace(next_run(scheduled, task), id);
                cv_.notify_one();
            } else {
                tasks_.erase(it);               // one-shot done
            }
        }

        // Notify under the lock. Once this lock is released, shutdown() can return and this object can be destroyed.
        --in_flight_;
        idle_cv_.notify_all();
    }

    static time_point next_run(time_point scheduled, const TaskState& task) {
        auto now = clock::now();
        if (!task.fixed_rate)
            return now + task.interval;

        // The first slot on the original cadence which is still in the future
        auto n = std::max<int64_t>(1, ((now - scheduled) / task.interval) + 1);
        return scheduled + task.interval * n;
    }

    // --- scheduler state ---
    std::mutex mu_;
    std::condition_variable cv_;
    std::condition_variable idle_cv_;     // in_flight_ reached 0
    std::priority_queue<Node, std::vector<Node>, MinTime> heap_;
    std::unordered_map<uint64_t, std::shared_ptr<TaskState>> tasks_;
    uint64_t next_id_{1};                 // 0 = invalid_task
    int in_flight_{0};                    // Jobs handed to the pool, which aren't finished
    bool stopping_{false};
    std::once_flag join_once_;

    // --- external worker pool ---
    ThreadPool& pool_;

    // Declared last, so the thread starts after all the members above are ready
    std::jthread timer_;
};
