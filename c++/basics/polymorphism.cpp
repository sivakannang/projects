/***************************************  Polymorphism   ******************************************************* 
 *  One interface, many types. Two forms:
 *
 *  - static bind/Compile-time polymorphism → binding happens at compile time, based on the static type. ( ex : function overloading, operator overloading )
 *  - dynamic bind/Run-time polymorphism → binding happens at runtime, based on the dynamic type of the object.
 *
 *  - Static type → what the compiler knows (here Base*).
 *  - Dynamic type → the actual object in memory at runtime (here Derived).
 *
 *  - Upcast is always safe (Derived* → Base*).
 *  - Casting back (Base* → Derived*) is only safe if the object is actually a Derived.

 *  class Base {                  public: void show(){ std::cout << "Base\n"; } };
 *  class Derived : public Base { public: void show(){ std::cout << "Derived\n"; } };
 *
 *  Derived d;
 *  Base* b = &d;   // b has static type Base*, dynamic type points to Derived
 *  b->show();      // non-virtual → Base::show() chosen at compile time (static type = Base*)
 *                  // If show() was virtual, the call would dispatch to Derived::show() using dynamic type
 *  Base *p     = &d;                           // downcast always safe 
 *  Derived *d1 = static_cast<Derived *>(b);    // safe here (object is truly Derived) 
 *  Derived *d1 = dynamic_cast<Derived *>(b);   // also safe, and checked
 *
 *  Rule of thumb
 *   - Use dynamic_cast when you don’t prove the dynamic type; it safely returns nullptr (or throws on reference) if the cast is invalid.
 *   - Use static_cast only when you are 100% certain about the dynamic type (e.g., your own factory just created that exact derived).
 *
 *   | Cast           | Needs polymorphic base? | Failure (ptr) | Failure (ref)   | Cross-cast | Runtime cost |
 *   | -------------- | ----------------------- | ------------- | --------------- | ---------- | ------------ |
 *   | `static_cast`  | No                      | UB            | UB              | ❌          | None         |
 *   | `dynamic_cast` | **Yes**                 | `nullptr`     | `std::bad_cast` | ✅          | RTTI check   |
 *
 *  - Use `virtual` in the base to enable dynamic dispatch. Prefer `override` in the derived.
 *    Must match signature (+ cv/ref/noexcept); return type can be covariant.
 *
 *  - Calls through base pointers/references pick derived overrides at run time (dynamic binding).
 *    Without `virtual`, similarly named functions in the derived hide base versions (static binding).
 *
 *  - `= 0` ⇒ pure virtual ⇒ class becomes abstract. (A pure virtual destructor still needs a definition.)
 *
 *  - Always make base destructors `virtual` when deleting via base pointers.
 *
 *  - `virtual` member functions cannot be `static`. Non-member friend functions cannot be virtual.
 *    A member function can be both virtual and a friend of another class.
 *
 *  - Access control is checked at compile time on the base declaration used for the call.
 *    The derived class’s access level for the overrider does not restrict virtual calls.
 *
 *  - Virtual dispatch is suppressed inside constructors/destructors.
 *    Calls inside a base’s ctor/dtor use base implementations, not derived overrides.
 *
 *  - vptr/vtable are typical implementation details (not mandated by the standard).
 *      * vptr: hidden pointer inside each polymorphic object pointing to its vtable
 *      * vtable: table of function pointers for virtual functions
 *      * Virtual call → compiler uses vptr to find correct function at runtime
 *
 *
 * Ques : We have a base class with a virtual function and a derived class which inherited it, so if we create 5 objetcs, how many vptr and vtable will be created ?
 *
 * Ans  : 5 vptr and 2 vtable ( one vtable per class and one vptr per object/instance )
 *
 * Ques: What will be output ( assume it is 64 bit machine ) ?
 *
 * struct A {
 *	int data[2] {10, 20};
 *	virtual void f() {}
 * }
 * int main() {
 *	A a;
 *	int *arr = (int *)&a;
 *	std::cout << arr[2] << std::endl; 
 * }
 *
 * Ans : 10
 * 	As it having virtual function, compiler will insert a vptr as a hidden/first member. As it is 64 machine , pointer size will be 8 bytes.
 * 	So class size will be 16 bytes, so integer arr[2]  will point base address + 8 , which is data[0] , which is 10
*
* 
***************************************** Interview Questions and Answers *******************************************
*
*  1. What is polymorphism in C++?
*     - The ability to call derived class methods using base class pointers or references.
* 
*  2. What is the difference between static and dynamic polymorphism?
*     - Static polymorphism is resolved at compile time (e.g., function overloading), while dynamic is resolved at runtime (via virtual functions).
* 
*  3. What is a virtual function?
*     - A member function that can be overridden in derived classes and invoked polymorphically.
* 
*  4. What is the role of virtual destructors?
*     - Ensures proper cleanup of derived class resources when deleting via base pointer
*
*  5. What is object slicing?
*     - Object slicing happens when you copy/assign a Derived object into a Base object by value
*     - Upcasting via pointer/reference (no copy) does not slice:
*
*  6. Can constructors be virtual?
*    - No, constructors cannot be virtual in C++, but destructors should be virtual for polymorphic base classes.
*********************************************************************************************************************/
#include <iostream>
#include <memory>

struct MarketData { 
	virtual void parse() = 0; 
	virtual ~MarketData() = default; 
};
struct NseFeed : MarketData { 
	void parse() override { std::cout << "NseFeed::parse()" << std::endl; } 
	void nseSpecific()    { std::cout << "nseSpecific()" << std::endl; } 
};
struct BseFeed : MarketData { 
	void parse() override { std::cout << "NseFeed::parse()" << std::endl; } 
	void bseSpecific()  { std::cout << "bseSpecific()" << std::endl; } 
};

void process(MarketData& md) {
	md.parse(); // (1) always do polymorphic work

	// (2) branch to derived-only behavior safely
	if (auto* p = dynamic_cast<NseFeed*>(&md)) {
		p->nseSpecific();
	} else if (auto* p = dynamic_cast<BseFeed*>(&md)) {
		p->bseSpecific();
	} else {
		// optional: unknown MarketData subtype
	}
}

int main() {
	auto md = std::make_unique<NseFeed>();
	process(*md);
	return 0;
}

