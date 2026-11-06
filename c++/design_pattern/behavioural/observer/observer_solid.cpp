/************************************************ Observer design pattern *******************************
 *
 * g++ -std=c++17 observer_solid.cpp -o observer_solid
 *
 * Observer Pattern's intent is to define a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.
 *
 * GUI components
 * Event monitoring - linux epoll is a similar idea
 * Subscription
 * *****************************************************************************************************/


#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

// ---------- Generic Observer / Subject ----------
template<class T>
struct IObserver {
	virtual void update(const T& v) = 0;
	virtual ~IObserver() = default;
};

template<class T>
class Subject {
	public:
		Subject() = default;
		// A copy would hold observers which detach only from the original, and then dangle
		Subject(const Subject&) = delete;
		Subject& operator=(const Subject&) = delete;

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
			BatteryLevel level{pct};
			if (cur_ == level) return;
			cur_ = level;
			notify_all(level);
		}
	private:
		std::optional<BatteryLevel> cur_;   // Empty until the first reading, so a first reading of 0 is still notified
};

class WifiSubject : public Subject<WifiStatus> {
	public:
		void set_status(WifiStatus s) {
			if (cur_ == s) return;
			cur_ = s;
			notify_all(s);
		}
	private:
		std::optional<WifiStatus> cur_;
};

// ---------- Observer watching BOTH ----------
class PosDisplay : public IObserver<BatteryLevel>, public IObserver<WifiStatus> {
	public:
		// Depends on Subject<T> only, so any subject of these events can be passed in
		PosDisplay(Subject<BatteryLevel>& b, Subject<WifiStatus>& w) : battery_(b), wifi_(w) {
			battery_.attach(this);
			wifi_.attach(this);
		}
		~PosDisplay() {
			// RAII: auto-detach on destruction. The subjects must outlive this object.
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
		Subject<BatteryLevel>& battery_;
		Subject<WifiStatus>& wifi_;
};

// ---------- Demo ----------
int main() {
	BatterySubject battery;
	WifiSubject wifi;

	PosDisplay ui{battery, wifi}; // stack object, no smart pointers. Declared last, so destroyed before the subjects.

	battery.set_level(30);
	wifi.set_status({WifiState::Connecting, -70});
	battery.set_level(10);
	wifi.set_status({WifiState::Connected, -45});
}



/*****************************************************************
  Why this is SOLID ?

SRP: Subjects only manage state & notify; PosDisplay only renders.

OCP: Add new subjects/observers without changing existing classes.

LSP: Any class derived from Subject<T> can be passed to PosDisplay.

ISP: IObserver<T> has one method per event type; no fat interface.

DIP: PosDisplay depends on Subject<T> and IObserver<T>, not on BatterySubject or WifiSubject.
     main is the only place which knows the concrete classes.

Extras you can mention (if asked)

Lifetime: This sample has one rule to follow, the subjects must outlive the observers. Destroying a subject
first makes ~PosDisplay use a dangling reference. Subscription tokens holding a weak_ptr to shared subject
state remove that rule. observer.cpp uses weak_ptr to skip dead observers.

Threading: This sample is single threaded. With threads, lock to copy the list, then notify outside the lock.

Cost: The snapshot allocates on every notify, and the re-check is O(n^2) in observers. Fine for a few observers,
not for a hot path.

Testing: Derive a FakeBattery from Subject<BatteryLevel>, call notify_all from it, and assert the UI output.

 *************************/
