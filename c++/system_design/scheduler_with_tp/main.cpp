// g++ -std=c++20 -O2 -Wall -Wextra -pthread main.cpp -o scheduler      ( std::jthread needs C++20 )

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "scheduler.hpp"

using namespace std::chrono_literals;

static const auto g_start = std::chrono::steady_clock::now();

static void log(const std::string& msg) {
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_start).count();
	std::cout << ("[" + std::to_string(ms) + " ms] " + msg + "\n");
}

int main() {
	ThreadPool pool(3);      // Declared first, as it must outlive the scheduler
	Scheduler scheduler(pool);

	scheduler.schedule_after(100ms, [] { log("one-shot : after 100ms"); });
	scheduler.schedule_at(Scheduler::clock::now() + 200ms, [] { log("one-shot : at now+200ms"); });

	// A slow task doesn't delay the others, which is the point of using a pool
	uint64_t slow = scheduler.schedule_every(300ms, [] {
		log("every 300ms, takes 500ms : start");
		std::this_thread::sleep_for(500ms);
	});

	uint64_t rate = scheduler.schedule_every(100ms, [] { log("every 100ms, fixed rate"); });
	uint64_t delay = scheduler.schedule_every(100ms, [] {
		log("every 100ms + 50ms of work, fixed delay");
		std::this_thread::sleep_for(50ms);
	}, /*fixed_rate*/false);

	std::cout << "schedule_every(0) returns invalid_task : "
		<< (scheduler.schedule_every(0ms, [] {}) == Scheduler::invalid_task) << "\n";

	std::this_thread::sleep_for(1s);

	std::cout << "cancel : " << scheduler.cancel(rate) << " " << scheduler.cancel(delay) << " " << scheduler.cancel(slow) << "\n";
	std::cout << "cancel again : " << scheduler.cancel(rate) << "\n";

	scheduler.shutdown();    // Waits for any running job
	log("shutdown complete");

	std::cout << "schedule after shutdown returns invalid_task : "
		<< (scheduler.schedule_after(0ms, [] {}) == Scheduler::invalid_task) << "\n";

	return 0;
}
