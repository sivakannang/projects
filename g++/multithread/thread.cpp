
/*********************************************************************************************************************
 * std::thread is the thread class that represents a single thread in C++.
 * To start a thread we simply need to create a new thread object and pass the executing code to be called (i.e, a callable object) into the constructor of the object.
 * Once the object is created a new thread is launched which will execute the code specified in callable
 *
 * A callable can be either of the three
 * 	- A function pointer
 * 	- A function object
 * 	- A lambda expression
 * 
 *
 * https://thispointer.com/c11-tutorial/
 *
 *
 * std::thread t()
 * t.joinable()
 * t.join()
 * t.detach()
 * t.get_id()
 *
 * std::this_thread::sleep_for(time)
 * std::this_thread::sleep_until ( std::chrono::system_clock::now() + std::chrono::seconds(10))
 * std::this_thread::get_id()
 * std::this_thread::yield()
 *
 * std::ref
 * std::cref
 *
 * std::mutex m;
 * std::recursive_mutex m;             ( std::recursive_mutex can be acquired several times by the same thread )
 * std::timed_mutex m;
 * std::recursive_timed_mutex m;
 * std::scoped_lock()
 *
 * m.lock()
 * m.unlock()
 * m.try_lock()
 *
 * m.try_lock_for()
 * m.try_lock_until()
 *
 * std::lock_guard<std::mutex> smart_lock(mutex)
 * std::lock_guard<std::recursive_mutex> smart_lock<mutex>
 *
 * std::scoped_lock() vs std::lock_guard() vs std::unique_lock()
 *
 *
 *
 * std::once_flag flag
 * std::call_once(flag, function, args...) || std::call_once(flag, lambda)
 *
 *
 *
 *
 *
 *
 *
 *
 * 	*/



#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <chrono>
#include <pthread.h>


class Mutex {
	private:
		pthread_mutex_t m_mutex;
	public:
		Mutex() { pthread_mutex_init(&m_mutex, 0);}
		
		~Mutex() { pthread_mutex_destroy(&m_mutex);}

		void lock() {
			pthread_mutex_lock(&m_mutex);
		}

		void unlock() {
			pthread_mutex_unlock(&m_mutex);
		}
};

int print_in_loop(int num, bool& status) {
	for ( int i = 0; i < num; ++i)
		std::cout << "print in function -> " << i << std::endl;
	status = true;
}

class FuncObject {
	public:
		void operator() (int num) {
			for ( int i = 0; i < num; ++i)
				std::cout << "print in FuncObject -> " << i << std::endl;
		}
};

void test_callable() {

	bool status = false;
	std::cout << "Invoking threads...." << std::endl;

	auto f = [] ( int num) {
		for ( int i = 0; i < num; ++i)
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

std::mutex g_mutex;
std::list<int> g_list;

void add_to_list(int begin, int end) {

	std::lock_guard<std::mutex> smart_lock(g_mutex);

	//g_mutex.lock();

	while ( begin <= end ) {
		g_list.push_back(begin++);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(1));
	}

	//g_mutex.unlock();
}

void print_list() {

	
	std::lock_guard<std::mutex> smart_lock(g_mutex);
	
	//g_mutex.lock();
	
	for ( auto it = g_list.begin(); it != g_list.end(); it++ )
		std::cout << *it << std::endl;

	//g_mutex.unlock();
}

void test_smart_lock() {

	std::thread t1(add_to_list, 1, 5);
	std::thread t2(add_to_list, 11, 15);
	std::thread t3(print_list);
	//t1.detach();
	//t2.detach();
	//t3.detach();
	t1.join();
	t2.join();
	t3.join();
}


struct Complex {

	//std::mutex mutex;
	std::recursive_mutex mutex;
	int i;

	Complex() : i(1) {}

	void mul(int x){
		std::lock_guard<std::recursive_mutex> lock(mutex);
		i *= x;
	}

	void div(int x){
		std::lock_guard<std::recursive_mutex> lock(mutex);
		i /= x;
	}

	void both(int x, int y){
		std::lock_guard<std::recursive_mutex> lock(mutex);
		mul(x);
		div(y);
	}
};

void test_recursive_mutex() {

	Complex comp;
	comp.mul(10);
	comp.div(2);
	comp.both(5, 4); // <- dead lock, due to same thread call lock second time. for avoid this use recursive_mutex instead of mutex, which allows same thread to acquire locks recursively

}


int main() {

	//test_callable();

	//test_smart_lock();

	//test_recursive_mutex();
	std::cout << "std::thread::hardware_concurrency() : " << std::thread::hardware_concurrency() << std::endl ;
	return 0;
}
