/************************************************ Observer design pattern *******************************
 * 
 * Observer Pattern's intent is to define a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.
 *
 * GUI components
 * Event monitoring - linux epoll
 * Subscribtion
 * *****************************************************************************************************/


#include <iostream>
#include <vector>
#include <algorithm>

// ---------- Generic Observer / Subject ----------
template<class T>
struct IObserver {
	virtual void update(const T& v) = 0;
	virtual ~IObserver() = default;
};

template<class T>
class Subject {
	public:
		void attach(IObserver<T>* o) {
			// optional dedupe
			if (std::find(observers_.begin(), observers_.end(), o) == observers_.end())
				observers_.push_back(o);
		}

		void detach(IObserver<T>* o) {
			observers_.erase(std::remove(observers_.begin(), observers_.end(), o),
					observers_.end());
		}

	protected:
		void notify_all(const T& value) {
			// Take a snapshot so observers_ can be modified (detach) during callbacks
			auto snap = observers_;
			for (auto* o : snap) {
				// Only notify if still attached
				if (std::find(observers_.begin(), observers_.end(), o) != observers_.end())
					o->update(value);
			}
		}

	private:
		std::vector<IObserver<T>*> observers_;
};

// ---------- Strong types ----------
struct BatteryLevel {
	int pct{0};
	bool operator==(const BatteryLevel& o) const { return pct == o.pct; }
};

enum class WifiState { Disconnected, Connecting, Connected };

struct WifiStatus {
	WifiState state{WifiState::Disconnected};
	int rssi{-127};
	bool operator==(const WifiStatus& o) const { return state==o.state && rssi==o.rssi; }
};

// ---------- Subjects ----------
class BatterySubject : public Subject<BatteryLevel> {
	public:
		void set_level(int pct) {
			BatteryLevel next{ std::max(0, std::min(100, pct)) };
			if (next == level_) return;
			level_ = next;
			notify_all(level_);
		}
	private:
		BatteryLevel level_{-1};
};

class WifiSubject : public Subject<WifiStatus> {
	public:
		void set_status(WifiStatus s) {
			if (s == cur_) return;
			cur_ = s;
			notify_all(cur_);
		}
	private:
		WifiStatus cur_{};
};

// ---------- Observer watching BOTH ----------
class PosDisplay : public IObserver<BatteryLevel>, public IObserver<WifiStatus> {
	public:
		PosDisplay(BatterySubject& b, WifiSubject& w) : battery_(b), wifi_(w) {
			battery_.attach(this);
			wifi_.attach(this);
		}
		~PosDisplay() {
			// RAII: auto-detach on destruction
			battery_.detach(this);
			wifi_.detach(this);
		}

		void update(const BatteryLevel& bl) override {
			std::cout << "[UI] Battery " << bl.pct << "%"
				<< (bl.pct<=10 ? " (LOW)" : "") << "\n";
		}
		void update(const WifiStatus& ws) override {
			const char* name = ws.state==WifiState::Connected ? "Connected"
				: ws.state==WifiState::Connecting ? "Connecting"
				: "Disconnected";
			std::cout << "[UI] Wi-Fi " << name << "  RSSI=" << ws.rssi << " dBm\n";
		}

	private:
		BatterySubject& battery_;
		WifiSubject& wifi_;
};

// ---------- Demo ----------
int main() {
	BatterySubject battery;
	WifiSubject wifi;

	PosDisplay ui{battery, wifi}; // stack object, no smart pointers

	battery.set_level(30);
	wifi.set_status({WifiState::Connecting, -70});
	battery.set_level(10);
	wifi.set_status({WifiState::Connected, -45});
}



/*****************************************************************
  Why this is SOLID ?

SRP: Subjects only manage state & notify; PosDisplay only renders.

OCP: Add new subjects/observers without changing existing classes.

LSP: Any subject obeying ISubject<T> can replace another.

ISP: Two small observer interfaces; no fat dependencies.

DIP: main composes abstractions; high-level doesn’t know concrete drivers.

Extras you can mention (if asked)

Production safety: Use RAII subscriptions (Subscription tokens) or weak_ptr to avoid dangling observers.

Threading: Snapshot observers, notify outside locks; use shared_mutex if needed.

Testing: Inject MockBatterySubject and drive set_level to assert UI output—DIP makes mocking trivial.

 *************************/
