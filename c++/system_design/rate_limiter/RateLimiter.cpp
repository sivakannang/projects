/*
   g++ -std=c++17 -O2 -Wall -Wextra -pthread RateLimiter.cpp -o rate_limiter

   Fixed window : count requests in each window of time, such as 5 per 10 seconds.
     Simple, and uses little memory. But it allows up to twice the limit around a boundary :
     5 requests at the end of one window, and 5 more at the start of the next.

   Token bucket : tokens refill at a steady rate up to a capacity, and each request takes one.
     Allows a burst up to the capacity, and limits the long term rate. No boundary problem.

   Sliding window log / counter : fixes the boundary problem of the fixed window, for more memory or some approximation.

   In an interview, be ready to talk about :
     - Both limiters here keep an entry per key forever, so memory grows with every new key.
       A real one removes idle entries, with a periodic sweep or an LRU.
     - The map lock is shared by all keys. Split the map into shards to reduce contention.
     - With many servers the count must be shared, such as in Redis, with an atomic update.
*/

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>


class FixedWindowRateLimiter {
	public:
		FixedWindowRateLimiter(int max_requests, std::chrono::seconds window_size)
			: max_requests_(max_requests), window_size_(window_size) {
				if (window_size_ <= std::chrono::seconds::zero())
					throw std::invalid_argument("window_size must be positive");
			}

		bool allowRequest(const std::string& key) {
			// Use steady_clock for monotonic time
			auto now = std::chrono::steady_clock::now();
			auto current_window = now.time_since_epoch() / window_size_;

			std::lock_guard<std::mutex> lock(mutex_);
			auto& entry = counters_[key];

			if (entry.window != current_window) {
				entry.window = current_window;
				entry.count = 0;
			}

			if (entry.count < max_requests_) {
				entry.count++;
				return true;
			}
			return false;
		}

	private:
		struct Counter {
			// The number of the current window, counted from the clock's start
			std::chrono::steady_clock::rep window = 0;
			int count = 0;
		};

		const int max_requests_;
		const std::chrono::seconds window_size_;
		std::unordered_map<std::string, Counter> counters_;
		std::mutex mutex_;

};


class TokenBucket {
	public:
		TokenBucket(double capacity, double refill_rate)
			: capacity_(capacity),
			tokens_(capacity), // Start with a full bucket
			refill_rate_(refill_rate),
			last_refill_time_(std::chrono::steady_clock::now()) {
				if (capacity < 0 || refill_rate < 0)
					throw std::invalid_argument("capacity and refill_rate can't be negative");
			}

		bool consume(double amount = 1.0) {
			std::lock_guard<std::mutex> lock(mutex_);
			refill();

			if (tokens_ >= amount) {
				tokens_ -= amount;
				return true;
			}
			return false;
		}

	private:
		void refill() {
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = now - last_refill_time_;

			// Add tokens based on time elapsed since last refill
			double new_tokens = elapsed.count() * refill_rate_;
			tokens_ = std::min(capacity_, tokens_ + new_tokens);
			last_refill_time_ = now;
		}

		double capacity_;
		double tokens_;
		double refill_rate_; // Tokens per second
		std::chrono::steady_clock::time_point last_refill_time_;
		std::mutex mutex_;
};

class TokenBucketRateLimiter {
	public:
		// capacity: Max burst size
		// rate: Tokens refilled per second
		TokenBucketRateLimiter(double capacity, double rate)
			: default_capacity_(capacity), default_rate_(rate) {}

		bool allowRequest(const std::string& key) {
			return bucket_for(key).consume(1.0);    // Uses only this key's lock, so other keys aren't blocked
		}

	private:
		TokenBucket& bucket_for(const std::string& key) {
			std::lock_guard<std::mutex> lock(map_mutex_);

			// Lazy initialization of a bucket for each unique key. TokenBucket has a mutex, so it can't be
			// moved, and try_emplace builds it in place. References stay valid, as nothing is ever erased.
			return buckets_.try_emplace(key, default_capacity_, default_rate_).first->second;
		}

		double default_capacity_;
		double default_rate_;
		std::mutex map_mutex_;
		std::unordered_map<std::string, TokenBucket> buckets_;
};

int main() {

	FixedWindowRateLimiter limiter(5, std::chrono::seconds(10));
	std::string user = "user123";

	for (int i = 1; i <= 7; ++i) {
		if (limiter.allowRequest(user))
			std::cout << "Request " << i << " allowed\n";
		else
			std::cout << "Request " << i << " blocked\n";
	}

	// Allows burst of 5, refills at 2 tokens per second
	TokenBucketRateLimiter tbRateLimiter(5.0, 2.0);
	user = "user_1";

	for (int i = 1; i <= 10; ++i) {
		if (tbRateLimiter.allowRequest(user)) {
			std::cout << "Request " << i << " allowed\n";
		} else {
			std::cout << "Request " << i << " blocked\n";
		}
		// Simulate small delay between requests
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return 0;
}
