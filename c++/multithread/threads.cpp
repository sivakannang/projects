
/*********************************************************************************************************************
 * std::thread is a class that represents a single thread of execution.
 *
 * To start a thread we simply need to create a new thread object and pass the executing code to be called (i.e, a callable object)
 *
 * A callable can be either of the three
 * 	- function
 * 	- lambda
 * 	- functor
 *
 *
 * std::thread is not copyable but moveable
 *
 * std::thread t()
 * t.joinable()
 * t.join()
 * t.detach()
 * t.native_handle();
 * t.get_id()
 *
 * std::this_thread::sleep_for(time)
 * std::this_thread::sleep_until ( std::chrono::system_clock::now() + std::chrono::seconds(10))
 * std::this_thread::get_id()
 * std::this_thread::yield()
 * std::thread::hardware_concurrency()  -> Returns the number of concurrent threads supported by the implementation
 * std::this_thread::get_id()
 *
 * std::ref
 * std::cref
 *
 * Either a thread should call join() or detach() before thread's destructor invokes, else threads destructor will crash
 * calling join() or detach() twice, will lead to crash
 *
 * std::thread doesn't return the return value of callable object, so we need to std::promise and std::future to get/set return values or exceptions
 * std::promise<int> promise
 * get -> promise.get_future()
 * set -> promise.set_value(), promise.set_exception(), promise.set_value_at_thread_exit(), promise.set_exception_at_thread_exit()
 *
 * std::future<int> future
 * get   -> future.get()
 * state -> valid() , wait() , wait_for() , wait_until()
 *
 *
 *
 * std::async vs std::thread [ std::async runs std::thread internally, it returns return value or exceptions. it supports lazy start ]
 *
 * std::async launch policies 
 *   1. std::launch::async     ( immediate start )
 *   2. std::launch::deferred  ( lazy call )
 *   3. std::launch::async | std::launch::deferred     << this is the default one, if we don't specify launch explicitly
 *
 *   std::future<int> fut = std::async(std::launch::async, count());
 *   int ret = fut.get();
 *
 * std::jthread
 *    - no need manual invoke of join/detach, which is automatically invoked in jthread's destrutor
 *    - provieds thread cancellation future
 *    - std::jthread::request_stop(), std::jthread::stop_requested(), std::stop_stoken
 *
 * std::packaged_task<int(int, int)> task
 * get   -> task.get_future()
 * fn()  -> task.valid() , task.swap()
 *
 * std::once_flag flag
 * std::call_once(flag, function, args...) || std::call_once(flag, lambda)
 *
 * 	*/



#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <string>
#include <sstream>


std::string generate_random_string(int32_t len) {

	std::stringstream s;
	static char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int32_t i = 0; i < len; i++)
		s << alphanum[rand() % (sizeof(alphanum) - 1)];
	std::cout << s.str() << std::endl;
	return s.str();
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
	auto producer = std::thread([&]() {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		promise.set_value("some_message");
		});

	auto future = promise.get_future();
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
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::microseconds(3));
	std::cout << "lazy_future() : thread invoked" << std::endl;
	std::cout << "lazy_future() : " << lazy_future.get() << std::endl;
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

void test_thread_join();
void test_thread_detach();

int main() {

	std::cout << "std::thread::hardware_concurrency() : " << std::thread::hardware_concurrency() << std::endl;

	//test_thread_creation();
	//test_future_promise();
	//test_async();
	test_packaged_task();

	return 0;
}

