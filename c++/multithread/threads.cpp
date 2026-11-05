
/*********************************************************************************************************************
 * g++ -std=c++20 -pthread threads.cpp -o threads      ( std::jthread needs C++20 )
 *
 * std::thread is a class that represents a single thread of execution.
 *
 * To start a thread we simply need to create a new thread object and pass the executing code to be called (i.e, a callable object)
 *
 * A callable can be any of these
 * 	- function
 * 	- lambda
 * 	- functor
 * 	- member function, with the object as the first argument ( std::thread t(&Worker::run, &worker) )
 *
 *
 * std::thread is not copyable but moveable
 *
 * std::thread t(callable, args...)     Note : 'std::thread t()' declares a function, it isn't a thread
 * t.joinable()
 * t.join()
 * t.detach()
 * t.native_handle();
 * t.get_id()
 *
 * std::this_thread::sleep_for(time)
 * std::this_thread::sleep_until ( std::chrono::steady_clock::now() + std::chrono::seconds(10))
 * std::this_thread::get_id()
 * std::this_thread::yield()
 * std::thread::hardware_concurrency()  -> Number of concurrent threads supported, can return 0 if it isn't known
 *
 * std::ref, std::cref                  -> Arguments are copied into the thread, so pass a reference with these
 *
 * A thread must be joined or detached before its destructor runs, else the destructor calls std::terminate
 * Calling join() or detach() again throws std::system_error. Check joinable() first.
 * An exception which escapes the thread's function calls std::terminate
 *
 * std::thread doesn't return the return value of callable object, so we need std::promise and std::future to get/set return values or exceptions
 * std::promise<int> promise
 * get -> promise.get_future()
 * set -> promise.set_value(), promise.set_exception(), promise.set_value_at_thread_exit(), promise.set_exception_at_thread_exit()
 *
 * std::future<int> future
 * get   -> future.get()      only once, then valid() is false. Rethrows the exception if one was set.
 * state -> valid() , wait() , wait_for() , wait_until()
 * std::shared_future         get() can be called many times, from many threads
 *
 *
 *
 * std::async vs std::thread [ std::async returns the return value or exception through a future. It supports lazy start. It may or may not create a new thread each time. ]
 *
 * std::async launch policies
 *   1. std::launch::async     ( runs as if on a new thread )
 *   2. std::launch::deferred  ( lazy, runs on the thread which calls get() or wait() )
 *   3. std::launch::async | std::launch::deferred     << this is the default one, the implementation chooses, so the task may never run if get() isn't called
 *
 *   std::future<int> fut = std::async(std::launch::async, count);
 *   int ret = fut.get();
 *
 *   Trap : the destructor of a future from std::async(std::launch::async, ...) blocks until the task ends.
 *          So 'std::async(std::launch::async, task);' with the future ignored, runs synchronously.
 *
 * std::jthread
 *    - no need of a manual join, the destructor calls request_stop() and join(). detach() is still available.
 *    - provides thread cancellation feature. It is cooperative, the thread must check the token.
 *    - std::jthread::request_stop(), std::stop_token::stop_requested(), std::stop_token
 *
 * std::packaged_task<int(int, int)> task
 * get   -> task.get_future()
 * fn()  -> task.valid() , task.swap()
 *
 * std::once_flag flag
 * std::call_once(flag, function, args...) || std::call_once(flag, lambda)
 *
 *
 * Mutex types :
 *
 *   std::mutex                     <mutex>          Basic. The first choice.
 *   std::recursive_mutex           <mutex>          Same thread can lock many times, must unlock the same number of times. Usually a design smell.
 *   std::timed_mutex               <mutex>          Adds try_lock_for(), try_lock_until()
 *   std::recursive_timed_mutex     <mutex>          Both of the above
 *   std::shared_mutex              <shared_mutex>   C++17. Many readers OR one writer. Useful when reads are long, and writes are few.
 *   std::shared_timed_mutex        <shared_mutex>   C++14. Same, with timeouts.
 *
 * Lock wrappers, all of them unlock in the destructor (RAII) :
 *
 *   std::lock_guard    C++11   Works with any mutex type. Locks once, no unlock(). Simple and small.
 *   std::unique_lock   C++11   Can lock and unlock, can be moved, timed locks, deferred locking. Needed for condition_variable.
 *   std::scoped_lock   C++17   Locks many mutexes at once without deadlock. Use it in place of lock_guard, even for one.
 *   std::shared_lock   C++14   Shared (reader) lock on a shared_mutex. The writer takes a unique_lock.
 *
 * std::condition_variable
 *    - wait(lock, predicate) sleeps until the predicate is true. The predicate handles spurious wakeups, they can still happen.
 *    - Change the condition while holding the mutex, else the wakeup can be lost. See threadpool.cpp.
 *
 * Avoid deadlock : lock in a consistent order, or use std::scoped_lock. Don't call unknown code while holding a lock.
 *
 *********************************************************************************************************************/



#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>


std::string generate_random_string(int32_t len) {

	// rand() isn't guaranteed to be thread safe, so each thread has its own generator
	thread_local std::mt19937 generator{std::random_device{}()};
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::uniform_int_distribution<std::size_t> pick(0, sizeof(alphanum) - 2);

	std::string s;
	for (int32_t i = 0; i < len; i++)
		s += alphanum[pick(generator)];
	return s;
}

int print_in_loop(int num, bool& status) {
	for (int i = 0; i < num; ++i)
		std::cout << "print in function -> " << i << std::endl;
	status = true;
	return 10;
}

class FuncObject {
public:
	void operator() (int num) {
		for (int i = 0; i < num; ++i)
			std::cout << "print in FuncObject -> " << i << std::endl;
	}
};

void test_thread_creation() {

	bool status = false;
	std::cout << "Invoking threads...." << std::endl;

	auto f = [](int num) {
		for (int i = 0; i < num; ++i)
			std::cout << "print in lambda -> " << i << std::endl;
	};

	FuncObject fo;

	std::thread t1(print_in_loop, 7, std::ref(status));
	std::thread t2(fo, 6);
	std::thread t3(FuncObject(), 6);
	std::thread t4(f, 5);

	std::cout << "Waiting for thread completion...." << std::endl;
	t1.join();
	t2.join();
	t3.join();
	t4.join();

	std::cout << "Thread execution completed " << status << std::endl;
}

void test_thread_errors() {

	std::thread t([]{});
	t.join();
	std::cout << "joinable after join : " << t.joinable() << std::endl;

	try {
		t.join();
	} catch (const std::system_error& e) {
		std::cout << "second join throws system_error : " << e.what() << std::endl;
	}
}

void dummy_function(std::promise<int>* promise) {
	int i = 5;
	std::cout << i << std::endl;
	promise->set_value(i);
	std::cout << i << std::endl;
}

void test_future_promise() {

	std::promise<int> promise_obj;
	std::future<int> future_obj = promise_obj.get_future();

	std::thread t(dummy_function, &promise_obj);
	std::cout << future_obj.get() << std::endl;
	t.join();

	auto promise = std::promise<std::string>();
	auto future = promise.get_future();

	auto producer = std::thread([&]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		promise.set_value("some_message");
		});

	auto consumer = std::thread([&]() {
		std::cout << future.get() << std::endl;
		});

	producer.join();
	consumer.join();
}


void test_async() {

	std::future<std::string> async_future = std::async(std::launch::async, generate_random_string, 10);


	std::cout << "async_future() : thread invoked" << std::endl;
	std::cout << (async_future.valid() ? "true" : "false") << std::endl;
	std::cout << "async_future() : " << async_future.get() << std::endl;
	std::cout << (async_future.valid() ? "true" : "false") << std::endl;

	std::future<std::string> lazy_future = std::async(std::launch::deferred, generate_random_string, 10);
	std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::microseconds(3));
	std::cout << "lazy_future() : not started yet, it runs inside get()" << std::endl;
	std::cout << "lazy_future() : " << lazy_future.get() << std::endl;

	std::future<int> failing = std::async(std::launch::async, []() -> int { throw std::runtime_error("failed in task"); });
	try {
		failing.get();
	} catch (const std::exception& e) {
		std::cout << "exception from async : " << e.what() << std::endl;
	}
}

void test_packaged_task() {

	std::packaged_task<std::string(int)> task1(generate_random_string);
	std::packaged_task<std::string(int)> task2(generate_random_string);
	auto future1 = task1.get_future();
	auto future2 = task2.get_future();

	std::thread thread1(std::move(task1), 10);
	std::thread thread2(std::move(task2), 10);
	thread1.join();
	thread2.join();

	std::cout << future1.get() + future2.get() << std::endl;
}


void worker(std::stop_token st, int id) {
	while ( !st.stop_requested() ) {
		std::cout << "jthread " << id << " running..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "jthread " << id << " stop requested" << std::endl;
}

void jthread_test() {

	std::jthread t(worker, 1);
	std::this_thread::sleep_for(std::chrono::seconds(3));
	t.request_stop();   // The destructor would also do this, before it joins
}

int main() {

	std::cout << "std::thread::hardware_concurrency() : " << std::thread::hardware_concurrency() << std::endl;

	test_thread_creation();
	test_thread_errors();
	test_future_promise();
	test_async();
	test_packaged_task();
	jthread_test();

	return 0;
}
