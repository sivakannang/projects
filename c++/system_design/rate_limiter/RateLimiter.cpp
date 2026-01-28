#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>


class FixedWindowRateLimiter {
	public:
		FixedWindowRateLimiter(int max_requests, std::chrono::seconds window_size)
			: max_requests_(max_requests), window_size_(window_size) {}

		bool allowRequest(const std::string& key) {
			// Use steady_clock for monotonic time
			auto now = std::chrono::steady_clock::now();
			auto current_window = now.time_since_epoch() / window_size_;

			std::lock_guard<std::mutex> lock(mutex_);
			auto& entry = counters_[key];

			if (entry.window != current_window) {
				entry.window = current_window;
				entry.count = 1;
				return true;
			}

			if (entry.count < max_requests_) {
				entry.count++;
				return true;
			}
			return false;
		}

	private:
		struct Counter {
			// Stores the raw numeric tick count from the steady_clock
			std::chrono::steady_clock::duration::rep window = 0;
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
			last_refill_time_(std::chrono::steady_clock::now()) {}

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
			std::lock_guard<std::mutex> lock(map_mutex_);

			// Lazy initialization of a bucket for each unique key
			if (buckets_.find(key) == buckets_.end()) {
				buckets_.emplace(std::piecewise_construct,
						std::forward_as_tuple(key),
						std::forward_as_tuple(default_capacity_, default_rate_));
			}

			return buckets_.at(key).consume(1.0);
		}

	private:
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



