/*********************************** Decorator design pattern ****************************************
 *
 *  g++ -std=c++17 decorator.cpp -o decorator
 *
 *  Decorator pattern allows behavior to be added to an individual object, dynamically, by wrapping it
 *
 *  - A decorator has the same interface as the object it wraps, and holds that object
 *  - Each call goes to the wrapped object first, and the decorator adds its own part to the result
 *  - The wrapped object is never changed, so decorators can be stacked in any order
 *  - It avoids a class for every combination (AutoGearNavigation, AutoGearSound, ...)
 *
 *  Decorator vs Proxy : both wrap an object behind the same interface
 *    Decorator adds behavior, and the client chooses what to stack
 *    Proxy controls access to the object, and often creates the object itself
 *
 * **************************************************************************************************/

#include <iostream>
#include <memory>
#include <string>
#include <utility>


class ICar {

public:
	virtual ~ICar() { std::cout << __func__ << std::endl; }
	virtual std::string get_description() const = 0;
	virtual int get_cost() const = 0;

	void print_detail() const { std::cout << get_description() << " , cost = $ " << get_cost() << std::endl; }
};


class BasicModel : public ICar {

public:
	~BasicModel() { std::cout << __func__ << std::endl; }
	std::string get_description() const override { return "Basic model"; }
	int get_cost() const override { return 1000000; }
};


// Owns the wrapped car, so deleting the outermost decorator deletes the whole chain
class Decorator : public ICar {

private:
	std::unique_ptr<ICar> car;
public:
	explicit Decorator(std::unique_ptr<ICar> car) : car(std::move(car)) {  }
	~Decorator() { std::cout << __func__ << std::endl; }
	std::string get_description() const override { return car->get_description(); }
	int get_cost() const override { return car->get_cost(); }
};


class AutoGear : public Decorator {

public:
	using Decorator::Decorator;
	~AutoGear() { std::cout << __func__ << std::endl; }
	std::string get_description() const override { return Decorator::get_description() + " + AutoGear"; }
	int get_cost() const override { return Decorator::get_cost() + 200000; }
};


class Navigation : public Decorator {

public:
	using Decorator::Decorator;
	~Navigation() { std::cout << __func__ << std::endl; }
	std::string get_description() const override { return Decorator::get_description() + " + Navigation"; }
	int get_cost() const override { return Decorator::get_cost() + 100000; }
};


class PremiumSoundSystem : public Decorator {

public:
	using Decorator::Decorator;
	~PremiumSoundSystem() { std::cout << __func__ << std::endl; }
	std::string get_description() const override { return Decorator::get_description() + " + PremiumSoundSystem"; }
	int get_cost() const override { return Decorator::get_cost() + 50000; }
};


int main()
{
	std::unique_ptr<ICar> car = std::make_unique<BasicModel>();
	car->print_detail();

	car = std::make_unique<Navigation>(std::move(car));
	car->print_detail();

	car = std::make_unique<AutoGear>(std::move(car));
	car->print_detail();

	car = std::make_unique<PremiumSoundSystem>(std::move(car));
	car->print_detail();

	return 0;
}
