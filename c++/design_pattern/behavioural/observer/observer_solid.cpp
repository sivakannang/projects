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
#include <memory>

// ---------- Generic Observer / Subject ----------
template<class T>
struct IObserver {
    virtual void update(const T& v) = 0;
    virtual ~IObserver() = default;
};

template<class T>
class Subject {                         // replaces ISubject<T> + per-type boilerplate
public:
    void attach(const std::shared_ptr<IObserver<T>>& o) {
        // optional: dedupe
        for (auto& w : obs_) if (w.lock() == o) return;
        obs_.push_back(o);
    }
    void detach(const std::shared_ptr<IObserver<T>>& o) {
        obs_.erase(std::remove_if(obs_.begin(), obs_.end(),
                    [&](const std::weak_ptr<IObserver<T>>& w){ return w.lock() == o; }),
                   obs_.end());
    }
protected:
    // call this from concrete subjects when state changes
    void notify_all(const T& value) {
        std::vector<std::shared_ptr<IObserver<T>>> snap;
        snap.reserve(obs_.size());
        // build snapshot & clean expired
        for (auto it = obs_.begin(); it != obs_.end();) {
            if (auto sp = it->lock()) { snap.push_back(std::move(sp)); ++it; }
            else it = obs_.erase(it);
        }
        for (auto& o : snap) o->update(value);
    }
private:
    std::vector<std::weak_ptr<IObserver<T>>> obs_;
};

// ---------- Strong types ----------
struct BatteryLevel { int pct{0}; bool operator==(const BatteryLevel& o) const { return pct == o.pct; } };
enum class WifiState { Disconnected, Connecting, Connected };
struct WifiStatus { WifiState state{WifiState::Disconnected}; int rssi{-127};
    bool operator==(const WifiStatus& o) const { return state==o.state && rssi==o.rssi; } };

// ---------- Subjects using the generic base ----------
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
    void update(const BatteryLevel& bl) override {
        std::cout << "[UI] Battery " << bl.pct << "%" << (bl.pct<=10 ? " (LOW)" : "") << "\n";
    }
    void update(const WifiStatus& ws) override {
        const char* name = ws.state==WifiState::Connected ? "Connected"
                           : ws.state==WifiState::Connecting ? "Connecting" : "Disconnected";
        std::cout << "[UI] Wi-Fi " << name << "  RSSI=" << ws.rssi << " dBm\n";
    }
};

// ---------- Demo ----------
int main() {
    BatterySubject battery;
    WifiSubject wifi;

    auto ui = std::make_shared<PosDisplay>();   // shared_ptr observer
    battery.attach(ui);
    wifi.attach(ui);

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
