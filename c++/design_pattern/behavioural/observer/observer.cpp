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
#include <list>
#include <algorithm>

// Observer interface
class IObserver {
	public:
		virtual void update(const int battery_level) = 0;
		virtual ~IObserver() = default;
};

// Observable Subject
class Battery {
	private:
		int m_battery_level;
		std::list<IObserver *> observers;
		void notify_all() {
			for ( auto observer : observers)
				observer->update(m_battery_level);
		}
	public:
		void attach(IObserver& observer) { observers.push_back(&observer); }
		void detach(IObserver& observer) { observers.remove(&observer); }

		void set_battery_level(int battery_level) {
			m_battery_level = battery_level;
			notify_all();
		}
};

// List of observers
class Display : public IObserver {
	public:
		void update(const int battery_level) override { std::cout << "Display::update() received event -> " << battery_level << std::endl; }
};

class Printer : public IObserver {
	public:
		void update(const int battery_level) override { std::cout << "Printer::update() received event -> " << battery_level << std::endl; }
};

int main()
{
	Battery battery;
	Display display;
	Printer printer;

	battery.attach(display);
	battery.attach(printer);

	battery.set_battery_level(30);
	battery.set_battery_level(20);

	battery.detach(printer);

	battery.set_battery_level(10);

	return 0;
}
