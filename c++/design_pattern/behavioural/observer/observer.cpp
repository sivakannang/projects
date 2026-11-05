/************************************************ Observer design pattern *******************************
 *
 * g++ -std=c++17 observer.cpp -o observer
 *
 * Observer Pattern's intent is to define a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.
 *
 * GUI components
 * Event monitoring - linux epoll is a similar idea
 * Subscription
 *
 * Two lifetime problems to handle, both are use after free if ignored:
 *   An observer is destroyed while still attached -> hold weak_ptr, so a dead observer is skipped
 *   An observer detaches itself inside update()   -> iterate over a copy, as the current list node is erased
 *
 * An observer detached by another observer during a notification still gets that one event.
 * observer_solid.cpp shows the other approach, raw pointers with RAII detach.
 * *****************************************************************************************************/

#include <iostream>
#include <list>
#include <memory>

// Observer interface
class IObserver {
	public:
		virtual void update(int battery_level) = 0;
		virtual ~IObserver() = default;
};

// Observable Subject
class Battery {
	private:
		int m_battery_level = 0;
		std::list<std::weak_ptr<IObserver>> observers;

		void notify_all() {
			observers.remove_if([](const std::weak_ptr<IObserver>& o) { return o.expired(); });

			auto snapshot = observers;
			for ( const auto& weak : snapshot )
				if ( auto observer = weak.lock() )
					observer->update(m_battery_level);
		}
	public:
		void attach(const std::shared_ptr<IObserver>& observer) { observers.push_back(observer); }
		void detach(const std::shared_ptr<IObserver>& observer) {
			observers.remove_if([&](const std::weak_ptr<IObserver>& o) { return o.lock() == observer; });
		}

		void set_battery_level(int battery_level) {
			m_battery_level = battery_level;
			notify_all();
		}
};

// List of observers
class Display : public IObserver {
	public:
		void update(int battery_level) override { std::cout << "Display::update() received event -> " << battery_level << std::endl; }
};

class Printer : public IObserver {
	public:
		void update(int battery_level) override { std::cout << "Printer::update() received event -> " << battery_level << std::endl; }
};

int main()
{
	Battery battery;
	auto display = std::make_shared<Display>();
	auto printer = std::make_shared<Printer>();

	battery.attach(display);
	battery.attach(printer);

	battery.set_battery_level(30);
	battery.set_battery_level(20);

	battery.detach(printer);
	battery.set_battery_level(10);

	display.reset();                 // Destroyed without detach, the battery just skips it
	battery.set_battery_level(5);

	return 0;
}
