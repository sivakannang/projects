/*********************************************************************************************************************
 *
 * std::mutex m;
 * std::recursive_mutex m;             ( std::recursive_mutex can be acquired several times by the same thread )
 * std::timed_mutex m;
 * std::recursive_timed_mutex m;
 *
 * m.lock()
 * m.unlock()
 * m.try_lock()
 *
 * m.try_lock_for()
 * m.try_lock_until()
 *
 * std::mutex is usually not accessed directly since C++17
 * std::unique_lock, std::lock_guard, or std::scoped_lock (since C++17) manage locking in a more exception-safe manner
 * 
 * std::lock         ->
 * std::lock_guard   -> smart lock
 * std::unique_lock  -> moveable but not copyable
 * std::scoped_lock  ->
 * std::lock_guard<std::mutex> smart_lock(mutex)
 * std::lock_guard<std::recursive_mutex> smart_lock<mutex>
 *
 * 
 * how do avoid deadlock ?
 * 	*/


#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

class Counter {
private:
	int value;
public:
	explicit Counter(int val) : value(val) {}
	void increment() { ++value;  }
	void decrement() { --value;  }
	int get_value() const { return value;  }
};

class Wallet {
private:
	int balance;
	std::mutex mutex;
public:
	
	Wallet(int bal = 0) : balance(bal) {}
	bool debit(int amt) {
		std::lock_guard<std::mutex> lock_guard(mutex);
		//if (amt > balance)
			//return false;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		balance -= amt;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return true;
	}
	int credit(int amt) {
		std::lock_guard<std::mutex> lock_guard(mutex);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		balance += amt;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return balance;
	}
	int get_balance() const { return balance; }
};


void test_smart_lock() {
	Wallet wallet;
	std::vector<std::thread> threads;
	for (int i = 0; i < 25; i++) {
		threads.push_back(std::thread(&Wallet::credit, &wallet, 100));
		threads.push_back(std::thread(&Wallet::debit, &wallet, 100));
	}
	for (int i = 0; i < 25 * 2; i++) {
		threads.at(i).join();
	}
	std::cout << wallet.get_balance() << std::endl;
}

int main() {

	test_smart_lock();
	

	return 0;
}
