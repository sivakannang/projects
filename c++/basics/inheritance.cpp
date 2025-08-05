/************************   C++ Inheritance *****************************
* 
*  - Inheritance allows one class (derived) to acquire properties of another (base).
*
*  - Promotes code reuse and supports polymorphism.
*
*  - Access specifiers affect inheritance:
*    * public inheritance: public and protected members stay accessible.
*    * protected inheritance: public becomes protected.
*    * private inheritance: public and protected become private.
*    - If no access specifier is provided:
*      * struct defaults to public inheritance
*      * class defaults to private inheritance
*
*  - Types of inheritance: Single, Multiple, Multilevel, Hierarchical, Hybrid.
*
*  - Use virtual inheritance to avoid diamond problem.
*
**************************  Interview Questions and Answers ***********************
*
*  1. What is inheritance in C++?
*     - It allows a class to derive members from another class.
*
*  2. What is the difference between public, private, and protected inheritance?
*     - They define how base class members are accessible in derived class.
*
*  3. What is multiple inheritance?
*     - A derived class inherits from more than one base class.
*
*  4. What is the diamond problem?
*     - Ambiguity due to multiple inheritance from a common base. Solved using virtual inheritance.
*
*  5. What is the use of virtual base class?
*     - Ensures only one base subobject when inherited via multiple paths.
*
*  6. Can constructors and destructors be inherited?
*     - Constructors are not inherited, but destructors are called in reverse order.
*
***********************************************************************************************/


#include <iostream>

// Single Inheritance
class Vehicle {
	public:
		void move() { std::cout << "Vehicle moves\n"; }
};

class Car : public Vehicle {
	public:
		void honk() { std::cout << "Car honks\n"; }
};

// Multiple Inheritance
class GPS {
	public:
		void navigate() { std::cout << "Navigating...\n"; }
};

class SmartCar : public Car, public GPS {
	public:
		void autoDrive() { std::cout << "Autodriving enabled\n"; }
};

// Virtual Inheritance (Diamond Problem)
class A {
	public:
		void display() { std::cout << "From A\n"; }
};

class B : virtual public A {};
class C : virtual public A {};
class D : public B, public C {};

void inheritanceDemo() {
	Car myCar;
	myCar.move();
	myCar.honk();

	SmartCar mySmartCar;
	mySmartCar.move();
	mySmartCar.honk();
	mySmartCar.navigate();
	mySmartCar.autoDrive();

	D obj;
	obj.display();  // Resolved via virtual inheritance
}

int main() {
	inheritanceDemo();
	return 0;
}

