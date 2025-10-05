#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "thread_pool.hpp"

class Scheduler {
public:
    using clock      = std::chrono::steady_clock;   // monotonic
    using time_point = clock::time_point;
    using duration   = clock::duration;

    explicit Scheduler(ThreadPool& pool) : pool_(pool) {
        timer_ = std::jthread([this](std::stop_token st){ timer_loop(st); });
    }

    ~Scheduler() { shutdown(); }

    // One-shot at absolute time
    template <class Fn>
    uint64_t schedule_at(time_point tp, Fn fn) {
        return add_task(tp, duration::zero(), /*recurring*/false, /*fixed_rate*/true, std::move(fn));
    }

    // One-shot after delay
    template <class Fn>
    uint64_t schedule_after(duration d, Fn fn) {
        return schedule_at(clock::now() + d, std::move(fn));
    }

    // Recurring: fixed_rate=true (catch-up cadence) or false (fixed-delay after finish)
    template <class Fn>
    uint64_t schedule_every(duration interval, Fn fn, bool fixed_rate = true) {
        auto tp = clock::now() + interval;
        return add_task(tp, interval, /*recurring*/true, fixed_rate, std::move(fn));
    }

    bool cancel(uint64_t id) {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = tasks_.find(id);
        if (it == tasks_.end()) return false;
        it->second->cancelled.store(true, std::memory_order_relaxed);
        cv_.notify_one();         // nudge timer to skip if it’s next
        return true;
    }

    void shutdown() {
        if (timer_.joinable()) timer_.request_stop();
        {
            std::lock_guard<std::mutex> lk(mu_);
            shutting_down_ = true;
        }
        cv_.notify_all();
        if (timer_.joinable()) timer_.join();

        // clean up state
        std::lock_guard<std::mutex> lk(mu_);
        while (!heap_.empty()) heap_.pop();
        tasks_.clear();
    }

private:
    struct TaskState {
        std::function<void()> fn;
        time_point            next;
        duration              interval{};
        bool                  recurring {false};
        bool                  fixed_rate{true};
        std::atomic<bool>     cancelled{false};
    };

    using Node = std::pair<time_point, uint64_t>; // (when, id)
    struct MinTime {
        bool operator()(const Node& a, const Node& b) const noexcept {
            return a.first > b.first; // min-heap
        }
    };

    template <class Fn>
    uint64_t add_task(time_point tp, duration iv, bool recurring, bool fixed_rate, Fn fn) {
        auto st = std::make_shared<TaskState>();
        st->fn         = std::function<void()>(std::move(fn)); // requires copyable callable
        st->next       = tp;
        st->interval   = iv;
        st->recurring  = recurring;
        st->fixed_rate = fixed_rate;

        uint64_t id = next_id_.fetch_add(1, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lk(mu_);
            tasks_.emplace(id, st);
            heap_.emplace(tp, id);
        }
        cv_.notify_one();
        return id;
    }

    void timer_loop(std::stop_token st) {
        std::unique_lock<std::mutex> lk(mu_);
        for (;;) {
            // stop/idle logic
            if (heap_.empty()) {
                cv_.wait(lk, [&]{
                    return st.stop_requested() || shutting_down_ || !heap_.empty();
                });
                if (st.stop_requested() || shutting_down_) break;
            } else {
                auto next_tp = heap_.top().first;
                if (cv_.wait_until(lk, next_tp, [&]{
                        return st.stop_requested() || shutting_down_ || heap_.top().first != next_tp;
                    }))
                {
                    if (st.stop_requested() || shutting_down_) break;
                    continue; // earlier task arrived or condition changed
                }
            }

            // run due tasks
            auto now = clock::now();
            while (!heap_.empty() && heap_.top().first <= now) {
                auto [when, id] = heap_.top();
                heap_.pop();

                auto it = tasks_.find(id);
                if (it == tasks_.end()) continue;
                auto stp = it->second;

                if (stp->cancelled.load(std::memory_order_relaxed)) {
                    tasks_.erase(it);
                    continue;
                }

                // capture info before unlocking
                auto last_next  = stp->next;
                auto recurring  = stp->recurring;
                auto fixed_rate = stp->fixed_rate;
                auto iv         = stp->interval;

                // hand off execution to pool (unlock around work)
                pool_.enqueue([this, id, stp, last_next, recurring, fixed_rate, iv] {
                    try { stp->fn(); } catch (...) { /* log/swallow */ }

                    if (recurring && !stp->cancelled.load(std::memory_order_relaxed)) {
                        time_point next_tp;
                        if (fixed_rate) {
                            // keep cadence relative to last scheduled time (catch up if late)
                            auto now2 = clock::now();
                            // number of intervals elapsed since last_next, at least 1
                            auto n = std::max<int64_t>(1, ((now2 - last_next) / iv) + 1);
                            next_tp = last_next + iv * n;
                        } else {
                            // fixed-delay: schedule after finish
                            next_tp = clock::now() + iv;
                        }
                        {
                            std::lock_guard<std::mutex> lk(mu_);
                            stp->next = next_tp;
                            heap_.emplace(next_tp, id);
                        }
                        cv_.notify_one();
                    } else {
                        std::lock_guard<std::mutex> lk(mu_);
                        tasks_.erase(id); // one-shot done, or recurring cancelled after run
                    }
                });
            }
        }
    }

    // --- scheduler state ---
    std::mutex mu_;
    std::condition_variable cv_;
    std::priority_queue<Node, std::vector<Node>, MinTime> heap_;
    std::unordered_map<uint64_t, std::shared_ptr<TaskState>> tasks_;
    std::atomic<uint64_t> next_id_{1};
    std::jthread timer_;
    bool shutting_down_{false};

    // --- external worker pool ---
    ThreadPool& pool_;
};

