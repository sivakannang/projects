/************************************************ Observer design pattern *******************************
 * 
 * Observer Pattern's intent is to define a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.
 *
 * GUI components
 * Event monitoring - linux epoll
 * Subscribtion
 * *****************************************************************************************************/

#include <iostream>
#include <string>
#include <memory>
#include <list>
#include <algorithm>

class IObserver {
	public:
		virtual void update(const std::string &msg) = 0;
		virtual ~IObserver() { std::cout << __func__ << std::endl; }
};

class Subject {
	private:
		std::list<std::shared_ptr<IObserver>> observers;
		void notify_all(const std::string& msg) {
			for ( auto observer : observers)
				observer->update(msg);
		}
	public:
		~Subject() { std::cout << __func__ << std::endl; }
		void attach(std::shared_ptr<IObserver> observer) { observers.push_back(observer); }
		void detach(std::shared_ptr<IObserver> observer) { observers.remove(observer); }
		void set_battery_level(int battery_level)
		{
			int BAT_THREASOLD = 30;
			std::cout << "Current battery level : " << battery_level << std::endl;
			if ( battery_level < BAT_THREASOLD )
			{
				std::string msg = "LOW POWER : " + std::to_string(battery_level);
				notify_all(msg);
			}
		}
};

class Display : public IObserver {
	private:
		std::string cls;
	public:
		Display() { cls = __func__; }
		~Display() { std::cout << cls << "::" << __func__ << std::endl; }
		void update(const std::string& msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
};

class Audio : public IObserver {
	private:
		std::string cls;
	public:
		Audio() { cls = __func__; }
		~Audio() { std::cout << cls << "::" << __func__ << std::endl; }
		void update(const std::string& msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
};

int main()
{
	std::unique_ptr<Subject> subject = std::make_unique<Subject>();
	std::shared_ptr<Display> display = std::make_shared<Display>();
	std::shared_ptr<Audio> audio     = std::make_shared<Audio>();

	subject->attach(audio);
	subject->attach(display);

	subject->set_battery_level(50);
	subject->set_battery_level(40);
	subject->set_battery_level(30);
	subject->set_battery_level(20);

	subject->detach(audio);

	subject->set_battery_level(10);

	return 0;
}
