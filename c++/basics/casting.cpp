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
*    5. any_cast<>()

*  1. static_cast<>()
*     - Performs compile-time type conversions between compatible types
*     - Numeric conversions
*     - Pointer conversions – between related classes in an inheritance hierarchy (upcast safe, downcast requires caution )
*     - Cannot cast away const/volatile qualifiers (use const_cast for that).
* 
*  2. dynamic_cast<>()
*     - Performs runtime-checked casting, primarily for safe downcasting in polymorphic types
*     - Requires at least one virtual function in base class.
*     - If cast fails returns nullptr for pointers, throws std::bad_cast for references.
*     - Slight runtime cost for type checking via RTTI (Run-Time Type Information).
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
*
*  5. any_cast<>()
*     - A type-erased holder for a single value of any copy-constructible type. It is both copyable and moveable.
*     - References aren’t stored, use std::reference_wrapper<T> if you must keep a reference
*     - Header: <any>. Core ops: has_value(), reset(), emplace<T>(...), type(), std::any_cast<T>().
*     - Performance: May heap-allocate; indirection + RTTI lookups
*     - When not to use: If the set of types is known → prefer std::variant. If only “maybe present” → std::optional<T>. If you need polymorphic dispatch → virtuals/concepts
*     - Decayed type: a = "hi"; stores const char[3], not std::string, keep that in mind when casting.
*	Extract without throwing: if (auto p = std::any_cast<T>(&a)) { ... } → nullptr on mismatch.
*
*	std::any a ;                                  // empty
*	a = 42;                                       // holds int (decayed)
*	std::cout << std::any_cast<int>(a)            //
*	a = <std::string>("siva");                    // holds string
*	std::cout << std::any_cast<std::string>(a)    //
*	bool ok = a.has_value();                      // true
*	const std::type_info& ti = a.type();          // typeid(std::string)
*	std::cout << a.type.name() << std::endl;
*	a.reset();                                    // empty again
*
*
*  Interview Questions:
*  --------------------
*  What is casting in C++?
*     - Converting one data type into another.
*  What is the difference between static_cast and dynamic_cast?
*     - static_cast is for compile-time checks; dynamic_cast supports runtime checking.
*  When would you use const_cast?
*     - To remove const-ness when required by non-const APIs.
*  Why is reinterpret_cast dangerous?
*     - It can break type safety and cause undefined behavior.
*  What happens if dynamic_cast fails?
*     - Returns nullptr for pointers, throws std::bad_cast for references.
*  What problem does std::any solve vs std::variant?
*     - Unknown set of types at compile time (fully type-erased, runtime checked). variant is for a known closed set
*
*
****************************************************************************************/

#include <iostream>
#include <typeinfo>

// Sample Classes for Casting
class MarketFeed {
	public:
		virtual void info() { std::cout << "Generic market feed\n"; }
		virtual ~MarketFeed() = default;
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

	//const_cast example
	const int x = 10;
	int& y = const_cast<int&>(x);
	y = 20;
	std::cout << "const_cast: " << x << " (Warning: Undefined behavior)\n";

	//reinterpret_cast example
	long p = reinterpret_cast<long>(&i);
	std::cout << "reinterpret_cast: " << p << "\n";

	delete feed1;
	return 0;
}

