/*********************************** Decorator design pattern ****************************************
 *
 *  Decorator pattern is a design pattern that allows behavior to be added to an individual object, dynamically
 *
 * **************************************************************************************************/

#include <iostream>
#include <string>
#include <memory>


class ICar {

public:
	std::string str;
	int cost;
public:
	virtual ~ICar() { std::cout << __func__ << std::endl; }
	virtual std::string get_description() = 0;
	virtual int get_cost() = 0;
	virtual void print_detail() = 0;

};


class BasicModel : public ICar {

public:
	std::string get_description() override { return str; }
        int get_cost() override { return cost; }
	void print_detail() override { std::cout << get_description() << " , cost = $ " << get_cost() << std::endl; }
	BasicModel() { str = "Basic model" ; cost = 1000000; }
   	~BasicModel() { std::cout << __func__ << std::endl; }
};


class Decorator : public ICar {

protected:
	ICar *car;
public:
	Decorator(ICar *car) : car(car) {  }
	virtual std::string get_description() = 0;
	virtual int get_cost() = 0;
	virtual void print_detail() = 0;
	virtual ~Decorator() { std::cout << __func__ << std::endl; }

};

class AutoGear : public Decorator {

public:
	AutoGear(ICar *car) : Decorator(car) { this->car->str = car->get_description() + " + AutoGear"; this->car->cost = car->get_cost() + 200000; }
	~AutoGear() { std::cout << __func__ << std::endl ; }
	std::string get_description() override { return car->str; }
        int get_cost() override { return car->cost; }
	void print_detail() override { std::cout << get_description() << " , cost = $ " << get_cost() << std::endl; }
};


class Navigation : public Decorator {

public:
	Navigation(ICar *car) : Decorator(car) { this->car->str = car->get_description() + " + Navigation"; this->car->cost = car->get_cost() + 100000; }
	~Navigation() { std::cout << __func__ << std::endl ; }
	std::string get_description() override { return car->str; }
        int get_cost() override { return car->cost; }
	void print_detail() override { std::cout << get_description() << " , cost = $ " << get_cost() << std::endl; }
};


class PremiumSoundSystem : public Decorator {

public:
	PremiumSoundSystem(ICar *car) : Decorator(car) { this->car->str = car->get_description() + " + PremiumSoundSystem"; this->car->cost = car->get_cost() + 50000; }
	~PremiumSoundSystem() { std::cout << __func__ << std::endl ; }
	std::string get_description() override { return car->str; }
        int get_cost() override { return car->cost; }
	void print_detail() override { std::cout << get_description() << " , cost = $ " << get_cost() << std::endl; }
};




int main()
{
	ICar *car = new BasicModel();
	car->print_detail();

	car = new Navigation(car);
	car->print_detail();

	car = new AutoGear(car);
	car->print_detail();

	car = new PremiumSoundSystem(car);
	car->print_detail();

	delete car;

	return 0;
}


