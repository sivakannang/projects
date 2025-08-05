/****----------------------------- Casting ---------------------------------
* 
*    upcast    -> derived to base
*    downcast  -> base to derived
* 
*  - Casting is the process of converting one data type to another.
*  - C++ offers four main types of casts:
*    1. static_cast<>()
*    2. dynamic_cast<>()
*    3. const_cast<>()
*    4. reinterpret_cast<>()

*  1. static_cast<>()
*     - Used for converting between compatible types (e.g., int to float, base to derived pointers).
*     - Checked at compile time.
* 
*  2. dynamic_cast<>()
*     - Used for safe downcasting in polymorphic types.
*     - Requires at least one virtual function in base class.
*     - If cast fails returns nullptr for pointers, throws std::bad_cast for references.
* 
*  3. const_cast<>()
*     - Removes const/volatile qualifiers.
*     - Useful when interfacing with legacy APIs.
*     - You are not allowed to const_cast variables that are actually declared as const. This results in undefined behavior.
*     - const_cast is used to remove the const-ness from references and pointers that ultimately refer to something that is not const
* 
*  4. reinterpret_cast<>()
*     - Low-level cast that converts between unrelated types.
*     - Dangerous: bypasses type safety and used for system-level operations.

*  Interview Questions:
*  --------------------
*  1. What is casting in C++?
*     - Converting one data type into another.
*  2. What is the difference between static_cast and dynamic_cast?
*     - static_cast is for compile-time checks; dynamic_cast supports runtime checking.
*  3. When would you use const_cast?
*     - To remove const-ness when required by non-const APIs.
*  4. Why is reinterpret_cast dangerous?
*     - It can break type safety and cause undefined behavior.
*  5. What happens if dynamic_cast fails?
*     - Returns nullptr for pointers, throws std::bad_cast for references.
*
*
****************************************************************************************/

#include <iostream>
#include <typeinfo>

// Sample Classes for Casting
class MarketFeed {
	public:
		virtual void info() { std::cout << "Generic market feed\n"; }
};

class NSEFeed : public MarketFeed {
	public:
		void info() override { std::cout << "NSE market feed\n"; }
		void nseSpecific() { std::cout << "NSE-specific behavior\n"; }
};

int main() {
	// static_cast example
	int i = 42;
	double d = static_cast<double>(i);
	std::cout << "static_cast: " << d << "\n";

	// dynamic_cast example
	MarketFeed* feed1 = new NSEFeed();
	NSEFeed* nse1 = dynamic_cast<NSEFeed*>(feed1);
	if (nse1) {
		nse1->nseSpecific();
	}

	MarketFeed* feed2 = new MarketFeed();
	NSEFeed* nse2 = dynamic_cast<NSEFeed*>(feed2);
	if (!nse2) {
		std::cout << "dynamic_cast failed on base object\n";
	}

	//const_cast example
	const int x = 10;
	int& y = const_cast<int&>(x);
	y = 20;
	std::cout << "const_cast: " << x << " (Warning: Undefined behavior)\n";

	//reinterpret_cast example
	long p = reinterpret_cast<long>(&i);
	std::cout << "reinterpret_cast: " << p << "\n";

	delete feed1;
	delete feed2;
	return 0;
}

