// g++ -std=c++17 casting.cpp -o casting

/****----------------------------- Casting ---------------------------------
 *
 *    upcast    -> derived to base
 *    downcast  -> base to derived
 *
 *  - Casting is the process of converting one data type to another.
 *  - C++ offers four cast operators:
 *    1. static_cast<>()
 *    2. dynamic_cast<>()
 *    3. const_cast<>()
 *    4. reinterpret_cast<>()
 *  - std::any_cast<>() is not one of these. It is a library function that extracts the value from a std::any.

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
 *     When dynamic cast not needed ?
 *      - If all the functions you need to call are declared virtual in the base class, you don’t need dynamic_cast
 *      - Because in that case, the base pointer/reference will automatically dispatch to the correct derived implementation at runtime (polymorphism)
 *     When dynamic_cast need ?
 *      - Call a method not present in the base, or
 *      - Check the actual runtime type of the object ( Example: dynamic_cast<SpecialFeed*>(ptr) to see if the object is really a SpecialFeed )
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
 *  std::any and std::any_cast<>()
 *     - A type-erased holder for a single value of any copy-constructible type. It is both copyable and moveable.
 *     - References aren’t stored, use std::reference_wrapper<T> if you must keep a reference
 *     - Header: <any>. Core ops: has_value(), reset(), emplace<T>(...), type(), std::any_cast<T>().
 *     - Performance: May heap-allocate; indirection + RTTI lookups
 *     - When not to use: If the set of types is known → prefer std::variant. If only “maybe present” → std::optional<T>. If you need polymorphic dispatch → virtuals/concepts
 *     - Decayed type: a = "hi"; stores const char*, not std::string, keep that in mind when casting.
 *	Extract without throwing: if (auto p = std::any_cast<T>(&a)) { ... } → nullptr on mismatch.
 *
 *	std::any a ;                                  // empty
 *	a = 42;                                       // holds int (decayed)
 *	std::cout << std::any_cast<int>(a)            //
 *	a = std::string("siva");                      // holds string
 *	std::cout << std::any_cast<std::string>(a)    //
 *	bool ok = a.has_value();                      // true
 *	const std::type_info& ti = a.type();          // typeid(std::string)
 *	std::cout << a.type().name() << std::endl;
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

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

class MarketData {
	public:
		virtual void parse() {
			std::cout << "MarketData::parse()" << std::endl;
		}
		virtual ~MarketData() = default;

		void print() { // Non-virtual (static binding when called via MarketData*)
			std::cout << "MarketData::print()" << std::endl;
		}
};

class NseFeed : public MarketData {
	public:
		void parse() override {
			std::cout << "NseFeed::parse()" << std::endl;
		}
		void print() { // hides base print()
			std::cout << "NseFeed::print()" << std::endl;
		}
		void nseSpecific() {
			std::cout << "NseFeed::nseSpecific()" << std::endl;
		}
};

class BseFeed : public MarketData {
	public:
		void parse() override {
			std::cout << "BseFeed::parse()" << std::endl;
		}
		void print() { // hides base print()
			std::cout << "BseFeed::print()" << std::endl;
		}
		void bseSpecific() {
			std::cout << "BseFeed::bseSpecific()" << std::endl;
		}
};

// -------------------- Raw pointer demo --------------------
void demo_dynamic_casting() {

	enum class Market { NSE, BSE };
	Market market = Market::NSE;

	MarketData* marketData = nullptr;
	if (market == Market::NSE) marketData = new NseFeed();
	if (market == Market::BSE) marketData = new BseFeed();
	if (!marketData) return;

	marketData->print(); // Non-virtual -> static binding based on *static type* (MarketData*)
	marketData->parse(); // Virtual -> dynamic dispatch based on *dynamic type* (actual object).
	// marketData->nseSpecific(); // Error: not in base interface

	// If you need derived-only APIs, use a safe downcast check with dynamic_cast.
	if (market == Market::NSE) {
		if (auto* f = dynamic_cast<NseFeed*>(marketData)) { f->nseSpecific(); } // NSE-only method
	} else if (market == Market::BSE) {
		if (auto* f = dynamic_cast<BseFeed*>(marketData)) { f->bseSpecific(); }
	}

	delete marketData; // manual cleanup


	std::unique_ptr<MarketData> marketdata = std::make_unique<BseFeed>();
	if ( auto *f = dynamic_cast<BseFeed *>(marketdata.get()) ) {
		f->print();
		f->parse();
		f->bseSpecific();
	}

	NseFeed nse;
	MarketData& market_data = nse;

	market_data.print();
	market_data.parse();

	try {
		NseFeed& nseFeed = dynamic_cast<NseFeed &>(market_data);
		nseFeed.print();
		nseFeed.parse();
		nseFeed.nseSpecific();
	}
	catch ( std::exception& ex) { std::cout << ex.what() << std::endl; }
}

// -------------------- Main driver --------------------
int main() {
	demo_dynamic_casting();
	// static_cast example
	int i = 42;
	double d = static_cast<double>(i);
	std::cout << "static_cast: " << d << "\n";

	// const_cast example: safe, because the object itself isn't const, only the reference to it.
	// If x were declared const int, writing through y would be undefined behavior.
	int x = 10;
	const int& ref = x;
	int& y = const_cast<int&>(ref);
	y = 20;
	std::cout << "const_cast: " << x << "\n";

	// reinterpret_cast example: uintptr_t is the integer type that can hold a pointer.
	// long can't on every system: it is 32 bits on 64-bit Windows, and this wouldn't compile there.
	std::uintptr_t p = reinterpret_cast<std::uintptr_t>(&i);
	std::cout << "reinterpret_cast: " << p << "\n";

	return 0;
}

