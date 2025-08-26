
/*
---------------------------------------------------------
   C++ Overview, OOPS, and Why C++?
--------------------------------------------------------- 


// What is C++:
// - Supports both procedural and object-oriented programming.
// - Provides low-level system access with high-level abstraction.
//
// OOPS Principles:
// - Encapsulation: Bundle data and functions.
// - Abstraction:   Expose essential features, hide implementation.
// - Inheritance:   allows a class (derived class) to acquire the properties and behaviors of another class (base class), enabling code reuse & polymorphism
// - Polymorphism:  enables the same code to invoke different behaviors depending on the object’s actual type at compile-time or runtime
//                  Many forms — function overloading, overriding.
//
// Why C++ over C:
// - C is procedural only; C++ supports OOP for better modularity.
// - STL, templates, exceptions, RAII enhance productivity.
// - Stronger type checking improves safety.
// - C++ suits large-scale, performance-critical systems.


Questions:

// 1. What is RAII and how does it help?
//    - RAII ties resource management to object lifetime (auto cleanup).
//    - resources like memory, files, locks, sockets are acquired in a constructor and released in the destructor
//    - makes resource management exception-safe and scope-bound

// 2. How does abstraction differ from encapsulation?
//    - Abstraction hides complexity; encapsulation hides data.


---------------------------------------------------------
   Function Overloading in C++
---------------------------------------------------------
   - Allows to define multiple functions with the same name but different parameter types or counts.
   - The correct function is selected at compile-time (static polymorphism).
   - Must differ in number or type of parameters, cannot differ by return type alone.

---------------------------------------------------------
   Default Arguments in C++
---------------------------------------------------------
   - Allow a function to be called with fewer arguments by automatically supplying predefined values for the missing ones
   - Default values must be provided from right to left.
   - Cannot redefine default arguments in the function definition if already defined in the declaration.
   - Commonly used in constructors, utility functions.

---------------------------------------------------------
   std::once_flag and std::call_once in C++
---------------------------------------------------------
   - Used to ensure a piece of code executes only once, even in multi-threaded environments.
   - std::once_flag : A flag object used to track whether a function has been called.
   - std::call_once(flag, callable, args...):
     • Executes the callable exactly once per flag.
     • Thread-safe and efficient for one-time initialization.

---------------------------------------------------------
   std::initializer_list
---------------------------------------------------------
   - It is a light weight, enable uniform initialization, immutable container ( supports begin(), end(), size() )
   - It is to pass a list of values to a function or constructors without needing arrays or vectors
   - It is kind of const array, we can't modify, need copy to other container for modifiction ( std::vector v(std::initializer_list) )

--------------------------------------------------------------
   std::bind and std::palceholder  ( avoid bind and prefer lambdas in modern c++ )
--------------------------------------------------------------
   - Creates a callable object by binding arguments to a function.
   - Useful for partial function application and callbacks.
   - auto bound_func = std::bind(callable, arg1, arg2, ..., std::placeholders::_1, ...);

   std::placeholders:
   - placeholders _1, _2, and upto _20, which are used with bind to defer some arguments, which will be supplied later when the bound function is invoked.
   - Placeholders tell std::bind which arguments to defer and in what position

----------------------------------------------------------------
   std::function C++11
----------------------------------------------------------------
   - legacy function pointer has limitations, it store only free functions
   - std::function is type-erased callable wrapper for free function, member function(via bind), functors, lambdas, bind.
   - Enables storing and passing around callable, it is copyable & assignable
   - std::function<ReturnType(ArgTypes...)> func = Callable

-----------------------------------------------------------------
   std::invoke:  ( C++17 )
-----------------------------------------------------------------
   - std::function is heavy weight, stores and invokes later not useful for instant dispatch, doesn't work directly with member functions or data pointers
   - It provides a unified way to call any callable object ( free function, member function, functor, lambdas, data members )
   - No overhead, instant dispatch, perfectly forwards arguments (supports std::move, std::ref, etc.)
   - std::invoke(callable, args...)

-----------------------------------------------------------------
   std::apply: ( C++17 )
-----------------------------------------------------------------
   - std::invoke calls a callable with already-separate arguments; std::apply unpacks a tuple-like and then uses invoke, supports pair, tuple, array.
   - supports perfect forwarding and internally calls invoke. Use std::forward_as_tuple or std::move into a tuple to control this
   	auto t = std::make_tuple(1, 2, 3, 4, 5);
	std::cout << std::apply(add, t) << std::cout;
   - How to avoid copies when elements are heavy or move-only?
	Use std::forward_as_tuple(...) to create a tuple of references, or std::tuple<...> with moved elements; then apply will forward correctly

--------------------------------------------------------------------------------
   std::optional<T> : (C++17)
--------------------------------------------------------------------------------
   - optional is a value that may be engaged (holds a T) or disengaged (no value).
   - std::optional<T> is type-safe, avoids sentinel hacks, avoids heap allocations, and makes APIs more expressive.
   - Core ops: has_value(), operator bool(), value(), value_or(default), emplace(...), reset(), *opt, opt->.
   - Construction: std::nullopt means “no value”. Prefer auto o = std::optional<T>{}; or std::optional<T> o = std::nullopt;.
   - No references: std::optional<T&> is not allowed; use std::optional<std::reference_wrapper<T>> if needed.
	std::optional<std::string> name;
	auto s = name.value_or("guest"); // no throw, returns default
   - When to prefer optional<T> vs variant<T, E>/expected<T,E>?
	If you only need “present/absent”, use optional. If you need error detail, use expected (C++23) or variant.

-------------------------------------------------------------------------------
   std::variant  C++17
-------------------------------------------------------------------------------
   - A type-safe discriminated union that holds exactly one of the listed alternatives (or is valueless_by_exception)
   - Access
	std::get<I>(v) / std::get<T>(v) (throws std::bad_variant_access on wrong alt)
	std::get_if<I/T>(&v) returns pointer or nullptr (no throw)
	std::holds_alternative<T>(v) checks active type
	v.index() gives active alternative’s index
   - Visitation: Use std::visit(visitor, v...) for pattern-matching style handling
   	
   	std::variant<int, std::string> v = 42;
	if (std::holds_alternative<int>(v))
        	std::cout << std::get<int>(v) << std::endl;
	v = std::string("hello");
	if (auto p = std::get_if<std::string>(&v))
		std::cout << *p << std::endl;

   - What is std::variant and how is it different from a union?
	A tagged union with automatic type tracking and safe access; unions lack active-member tracking and require manual discipline
   - What is std::visit doing under the hood?
	It dispatches to the correct overload based on the active alternative(s); it’s resolved at compile time via generated switch/overloads
   - variant vs any vs optional quick choice?
	Known closed set → variant; arbitrary unknown type → any; maybe-present single type → optional
   - What’s the difference between std::visit and std::get?
	get extracts a specific alternative; visit dispatches to code based on which alternative is active.

---------------------------------------------------------------------------------
   std::forward
---------------------------------------------------------------------------------
   - Perfectly forwards arguments (preserves lvalue/rvalue nature) and avoids unnecessary copies, preserves move semantics ( std::forward<T>(arg) )

   When std::forward Becomes Necessary ?
   
   - Let’s say your function doesn’t know whether it’s being given an lvalue or rvalue, and it’s just a middleman — like in:
   
    - Factory functions
    - Higher-order functions
    - Generic event dispatchers
    - STL algorithms or containers calling your function internally

    template <typename T, typename... Args>
    std::unique_ptr<T> make_my_object(Args&&... args) {
    	// We don't know if args are lvalues or rvalues
	// if we skip std::forward here, all args become lvalues inside and move constructors won't be used
    	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

   ✅ Rule of Thumb
   	- Direct call? → No std::forward.
	- Forwarding inside a template? → Yes, use std::forward


    - What is the difference between std::forward and std::move?
    	std::move unconditionally casts to rvalue (you lose lvalue nature).
	std::forward conditionally casts to rvalue only if the original argument was rvalue.

    - Why do we need std::forward if we already have std::move?
    	std::move breaks perfect forwarding by converting even lvalues to rvalues.
	std::forward preserves the original value category.
	
    - Can std::forward be used outside templates?
    	It’s possible but pointless — it’s intended for forwarding function arguments in templates.
	
    - How does std::forward help in implementing perfect forwarding?
    	It allows a function template to pass its arguments to another function without changing their lvalue/rvalue nature.
	
    - What will be the output of this code?
    	template <typename T>
	void relay(T&& x) {
    		f(std::forward<T>(x));
	}
	relay(lvalue) → calls lvalue overload of f
	relay(rvalue) → calls rvalue overload of f.
	
    - pitfall : What happens if you call std::forward with the wrong template type?
    	It can incorrectly cast an lvalue to rvalue (or vice versa) → potential dangling references or extra copies

    - Forwarding reference or not?
    	template <typename T>
    	void f(T&& x);         // This forwards reference only when T is deduced (in a template)

    	void g(std::string&&); //It is a plain rvalue reference overload. Forwarding semantics apply only to above code.


    - What’s the bug here?
	template <typename T>
	void call(T&& x) {
  		callee(std::forward<int>(x)); // WRONG, it must be std::forward<T>()
	}
	Answer: Passing a different template type to std::forward (int instead of T) can cast an lvalue to rvalue incorrectly → dangling/UB or extra moves.

    - What is the output, explain why?
	void h(const std::string&){ std::cout<<"L\n"; }
	void h(std::string&&){      std::cout<<"R\n"; }

	template <typename T>
	void wrap(T&& v){
		h(v);                     // (1)
		h(std::move(v));          // (2)
		h(std::forward<T>(v));    // (3)
	}

	std::string s = "x";
	wrap(s);             // call with lvalue
	wrap(std::string("y")); // call with rvalue

	Answer (calls):

	wrap(s) → (1) L, (2) R, (3) L (since T = std::string&, forward<T> keeps lvalue).

	wrap(std::string("y")) → (1) L (named param is lvalue), (2) R, (3) R (T = std::string, so forward casts to rvalue).


    - Why is std::forward needed in factories?
	template <class T, class... Args>
	auto make(Args&&... args){
  		return T(std::forward<Args>(args)...); // OK
  		// return T(args...);                  // BUG: loses rvalueness
	}
	Answer: Without std::forward, all args become lvalues, disabling moves and causing extra copies; std::forward perfectly forwards each arg’s original value category.


---------------------------------------------------------
   Lambda
---------------------------------------------------------

  - A lambda creates an unnamed unique closure type with an operator(). use auto or templates to hold it.

  - Captures define what outside variables become part of the closure’s state:
	[=] copy-captures all used locals; [&] reference-captures all used locals.
	[x] copy-captures x; [&x] ref-captures x; [x = expr] init-captures (can move: [p = std::move(ptr)], can std::forward).
	Capturing this copies the pointer; [*this] (C++17) copies the object.
	No-capture lambdas can decay to function pointers, captured lambdas cannot.

  - global, static global, static local, thread_int(static storage) members are accessible by default without capture

  - operator() is const by default. Use mutable to allow modifying captured-by-copy members inside the lambda.

  - Generic lambdas (C++14) use auto parameters; templated lambdas (C++20) can have an explicit template head.
    	auto add = [](auto x, auto y) { return x + y; };
	auto add = []<class T, class U>(T a, U b) { return a + b; };

  - Prefer passing lambdas as template callables over storing in std::function (avoids type erasure/allocations).

  - Syntax: [capture](parameters) -> return_type { body }; or [capture](parameters) mutable -> return_type { body };


---------------------------------------------------------
   chrono Library
---------------------------------------------------------

system_clock          → wall time (can jump with NTP/time-set)
steady_clock          → monotonic, doesn’t go backwards (use for measuring intervals).
high_resolution_clock → alias to the highest resolution (may equal one of the above).

Use std::steady_clock for measuring intervals/timeouts.
Use std::system_clock for calendar/wall-clock timestamps.

Measuring Elapsed Time (use steady_clock)
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(120ms);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "ms = " << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << std::endl;

---------------------------------------------------------
   Variadic Templates in C++
---------------------------------------------------------

What problem do variadic templates solve?
	Allow functions/classes to accept an arbitrary number of template parameters (types or values) with type safety at compile time.

How do you count pack elements?
	sizeof...(Ts) or sizeof...(args).

Why prefer fold expressions over recursion?
	- They’re simpler, compile faster, avoid template depth limits, and are less error-prone.
 	- Syntax: (... op args) or (args op ...)
	- Example: template<typename... Args> auto sum(Args... args) { return (... + args); }

-------------------------------------------------

 - how to execute something before invoke main() ?
	global/static object's construtor will be invoked before main

**********************************************/

#include <iostream>
#include <functional>
#include <utility>
#include <initializer_list>
#include <chrono>
#include <thread>

void printOrderDetails(int orderId, const std::string& product, double price)
{
    std::cout << "order_id - " << orderId << " , product - " << product << " , price - " << price << std::endl;
}

void print_all(std::initializer_list<int> nums) {
	for (int n : nums) std::cout << n << " ";
	std::cout << "";
}

class Printer {
public:
    void printMessage(const std::string& msg) const {
        std::cout << "Printer: " << msg << std::endl;
    }
};

class PriceBandCheck {
    double lower;
    double upper;

public:
    PriceBandCheck(double l, double u) : lower(l), upper(u) {}

    bool operator()(double price) const {
        return price >= lower && price <= upper;
    }
};

void bind_forward_demo() {

	bool flag = false;

	// std::initializer_list
	print_all({1, 2, 3, 4, 5});
	
	// std::function & std::invoke - free function
	std::function<void(int, std::string, double)> printOrder = printOrderDetails;
	printOrder(100, "TV", 40000);
	std::invoke(printOrderDetails, 100, "TV", 40000);

	// std::function & std::invoke - memeber function
	Printer printer;
	auto printmsg = std::bind(&Printer::printMessage, &printer, std::placeholders::_1);
	printmsg("siva");
	std::invoke(&Printer::printMessage, &printer, "siva");

	// std::function - bind with placeholder
	auto printorder = std::bind(printOrderDetails, std::placeholders::_1, "Laptop", std::placeholders::_2);
	printorder(101, 75999.99);

	// std::function & std::invoke - lambdas with capture
	std::function<int(int)> lambdaFunc = [flag](int x) {
        	return x + 10;
	};
	int result = lambdaFunc(15);
	result = std::invoke( [flag](int x) {
		return x + 10;
	}, 15);

	// std::function & std::invoke - functors
	PriceBandCheck checker(90, 110);
	std::function<bool(double)> fn = checker;
	std::cout << fn(100) << std::endl;
	std::cout << std::invoke(checker, 100) << std::endl;

}


void chrono_test() {
	using namespace std::chrono;

	auto start = steady_clock::now();
	std::this_thread::sleep_for(milliseconds(500));
	auto end = steady_clock::now();

	auto duration = duration_cast<milliseconds>(end - start);
	std::cout << "Elapsed time: " << duration.count() << " ms";
}

int main() {
	chrono_test();
	bind_forward_demo();
	return 0;
}
