/***************************************  Polymorphism   ******************************************************* 
*
*  - Polymorphism allows one interface to represent multiple underlying types and it has two types
*    * Compile-time (static): achieved using function overloading or operator overloading
*    * Run-time (dynamic): achieved using virtual functions and base class pointers/references
*
*  - Use `virtual` in base class to enable runtime polymorphism.
*  - Derived class must override with same signature for correct dispatch.
*  - Calls via base pointer use derived method if `virtual` is used.
*
*  - `= 0` makes a function pure virtual → class becomes abstract.
*  - Always make base class destructors `virtual` for safe cleanup.
*  - Without `virtual`, overriding leads to compile-time (static) binding.
*  - virtual function can not be static or friend function
*  - Using virtual Keyword, we can access the private method of derived class. Compiler checks for access specifier only at compile time
*
*  - The compiler uses a virtual pointer (`vptr`) and a virtual table (`vtable`) for runtime dispatch
*      * vptr is a hidden pointer inside each polymorphic object and it points to the class's `vtable`
*      * vtable is a table of function pointers for all virtual functions.
*      * When a virtual function is called, vptr is used to locate the correct function at runtime
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
*     - Ensures proper cleanup of derived class resources when deleting via base pointer.
*
*  5. What is object slicing?
*     - Occurs when a derived object is assigned to a base object by value, losing derived parts.
*
*  6. Can constructors be virtual?
*    - No, constructors cannot be virtual in C++, but destructors should be virtual for polymorphic base classes.
*********************************************************************************************************************/

#include <iostream>
#include <vector>

class MarketFeed {

	protected:
		std::string provider;
		std::string endpoint;

	public:
		virtual ~MarketFeed() {}

		void configure(const std::string& name, const std::string& url) {
			provider = name;
			endpoint = url;
		}

		void printConfig() const {
			std::cout << "Provider: " << provider << ", Endpoint: " << endpoint << std::endl;
		}

		virtual void connect() const = 0;

		virtual void fetch() const = 0;
};

class NSEFeed : public MarketFeed {
	
	public:
		void initialize() {
			configure("NSE", "nse.example.com:1234");
		}
		void connect() const override {
			std::cout << "Connecting to NSE feed at " << endpoint << "...";
		}
		void fetch() const override {
			std::cout << "Fetching market data from NSE...";
		}
};

class BSEFeed : public MarketFeed {
	
	public:
		void initialize() {
			configure("BSE", "bse.example.com:5678");
		}
		void connect() const override {
			std::cout << "Connecting to BSE feed at " << endpoint << "...";
		}
		void fetch() const override {
			std::cout << "Fetching market data from BSE...";
		}
};

void acquireMarketData(const std::vector<MarketFeed*>& feeds) {
	for (const auto* feed : feeds) {
		feed->printConfig();
		feed->connect();
		feed->fetch();
		std::cout << std::endl;
	}
}

int main() {
	
	NSEFeed nse;
	nse.initialize();

	BSEFeed bse;
	bse.initialize();

	std::vector<MarketFeed*> feeds = { &nse, &bse };
	acquireMarketData(feeds);

	return 0;
}

