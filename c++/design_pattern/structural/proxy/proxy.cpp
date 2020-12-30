/******************************* Proxy Design Pattern ***************************************
 *
 * Both the Proxy and the RealSubject implement the Subject interface. This allows any client to treat the Proxy just like the RealSubject.
 *
 * The RealSubject is usually the object that does most of the real work; the Proxy controls access to it.
 *
 * The control may be needed if the Subject is running on a remote machine, if the Subject is expensive to create in some way or if access to the subject needs to be protected in some way.
 *
 * The Proxy often instantiates or handle the creation of the RealSubject.
 *
 * The Proxy keeps a reference (pointer) to the Subject, so it can forward requests to the Subject when necessary.
 *
 * In some cases, the Proxy may be responsible for creating and destroying the RealSubject. Clients interact with the RealSubject through the Proxy.
 *
 ********************************************************************************************/

#include <iostream>
#include <string>
#include <memory>

// Base or Interface class
class ICar {

protected:
	int age;
public:
	virtual void drive() = 0;
	virtual ~ICar() { std::cout << __func__ << std::endl; }
};

// Concrete class
class Car : public ICar {

public:
	void drive() override { std::cout << "Driving now ...." << std::endl; }
	~Car() { std::cout << __func__ << std::endl; }
};

class ProxyCar : public ICar {

private:
	std::unique_ptr<ICar> real_car;
public:
	ProxyCar(int age = 0) { this->age = age; real_car = nullptr; }
	void drive() override
	{
		if ( age < 18 )
			std::cout << "Age is less than 18, can't drive" << std::endl;
		else
		{
			if ( real_car == nullptr )
				real_car = std::make_unique<Car>();   // lazy initialization, create on need
			real_car->drive();
		}
	}
	~ProxyCar() { std::cout << __func__ << std::endl; }
};


int main(int argc, char *argv[], char **env)
{
	std::unique_ptr<ICar> car = std::make_unique<ProxyCar>(15);
	car->drive();

	car = std::make_unique<ProxyCar>(21);
	car->drive();

	return 0;
}
