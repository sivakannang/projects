
#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <thread>
#include <mutex>

//https://medium.com/@gayashanbc/explore-the-observer-design-pattern-with-c-8592459b0486
//
// Observer design pattern -> Observer pattern is used when there is one-to-many relationship between objects such as if one object is modified, its depenedent objects are to be notified automatically

/*
 * Singleton
 * Factory method
 * Abstract factory
 * Object pool
 * Observer
 *
 *
 *
 */
class Singleton {

	private:

		Singleton() = default;
		~Singleton() = default;
		Singleton(const Singleton&) = default;
		Singleton& operator= (Singleton&) = default;

	public:
		static Singleton& getInstance() {
			static Singleton instance;
			return instance;
		}
};

class IObserver {
	public:
		virtual void update(std::string state) = 0;
};

class Observer : public IObserver {
	private:
		std::string state;
	public:
	void update ( std::string state ) override {
		this->state = state;
	}
	std::string get_state() const {
		return state;
	}
};

class Subject {
	private:
		std::string state;
		std::set<Observer *> observers;
	public:
		void subscribe(Observer *observer) {
			observers.insert(observer);
		}
		void unsubscribe(Observer *observer) {
			observers.erase(observer);
		}
		void publish() {
			for_each( observers.begin(), observers.end(), [this](Observer *observer) { observer->update(state); });
		}
		void set_state(std::string state) {
			this->state = state;
			publish();
		}
};



void test_observer_pattern() {

	Subject subject;
	Observer obs1, obs2, obs3;

	subject.subscribe(&obs1);
	subject.subscribe(&obs2);
	subject.subscribe(&obs3);

	subject.set_state("Price incresed by 100 RS");

	std::cout << obs1.get_state() << " " << obs2.get_state() << " " << obs3.get_state() << std::endl;
	subject.unsubscribe(&obs3);
	subject.set_state("Price incresed by 200 RS");
	std::cout << obs1.get_state() << " " << obs2.get_state() << " " << obs3.get_state() << std::endl;
}
int main() {

	Singleton& instance = Singleton::getInstance();

	test_observer_pattern();
	return 0;
}
