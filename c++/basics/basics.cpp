
/*
---------------------------------------------------------
   C++ Overview, OOPS, and Why C++?
--------------------------------------------------------- 


// What is C++:
// - General-purpose, multi-paradigm language developed by Bjarne Stroustrup as an extension of C.
// - Supports both procedural and object-oriented programming.
// - Provides low-level system access with high-level abstraction.
//
// OOPS Principles:
// - Encapsulation: Bundle data and functions.
// - Abstraction: Expose essential features, hide implementation.
// - Inheritance: Derive new classes from existing ones.
// - Polymorphism: Many forms — function overloading, overriding.
//
// Why C++ over C:
// - C is procedural only; C++ supports OOP for better modularity.
// - STL, templates, exceptions, RAII enhance productivity.
// - Stronger type checking improves safety.
// - C++ suits large-scale, performance-critical systems.


Questions:

// 1. What is the difference between C and C++?
//    - C is procedural; C++ adds object-oriented features.

// 2. Explain the four pillars of OOPS.
//    - Encapsulation, Abstraction, Inheritance, Polymorphism.

// 3. What is RAII and how does it help?
//    - RAII ties resource management to object lifetime (auto cleanup).

// 4. Why use C++ for system-level applications?
//    - It offers both low-level access and high-level abstractions.

// 5. How does abstraction differ from encapsulation?
//    - Abstraction hides complexity; encapsulation hides data.





---------------------------------------------------------
   Function Overloading in C++
---------------------------------------------------------

// Function overloading allows multiple functions with the same name
// but different parameter types or counts.
// The correct function is selected at compile-time (static polymorphism).

// Rules:
// - Must differ in number or type of parameters.
// - Cannot differ by return type alone.

// Use Cases:
// - Improve code readability.
// - Handle similar operations for different types.

Questions:

// 1. What is function overloading?
//    - Defining multiple functions with the same name but different parameter lists.

// 2. Can functions be overloaded by return type alone?
//    - No, the compiler cannot differentiate based on return type only.

// 3. What kind of polymorphism does overloading support?
//    - Compile-time polymorphism.

// 4. Is overloading allowed for constructors?
//    - Yes, constructors can be overloaded.

// 5. Why is function overloading useful?
//    - It simplifies interfaces for functions that perform similar actions.



---------------------------------------------------------
   Default Arguments in C++
---------------------------------------------------------

// Default arguments allow a function to be called with fewer arguments than it is declared with.
// The compiler supplies the default values when arguments are omitted.

// Rules:
// - Default values must be provided from right to left.
// - Cannot redefine default arguments in the function definition if already defined in the declaration.
// - Commonly used in constructors, utility functions.

Questions:

// 1. What are default arguments in C++?
//    - Values assigned to function parameters that the caller may omit.

// 2. Can we skip a middle argument and pass the next one?
//    - No, arguments must be passed from left to right; defaults apply from right to left.

// 3. Where can default arguments be defined?
//    - In the function declaration (preferably in header files).

// 4. Is it allowed to define default values in both declaration and definition?
//    - No, it causes a redefinition error.

// 5. What is a common use case for default arguments?
//    - Constructors with optional configuration values.


/* ---------------------------------------------------------
   std::once_flag and std::call_once in C++
--------------------------------------------------------- */

// Used to ensure a piece of code executes only once, even in multi-threaded environments.
// Provided by <mutex> header.

// - std::once_flag:
//   • A flag object used to track whether a function has been called.

// - std::call_once(flag, callable, args...):
//   • Executes the callable exactly once per flag.
//   • Thread-safe and efficient for one-time initialization.

Questions:

// 1. What is std::once_flag used for?
//    - To coordinate one-time execution in multi-threaded code.

// 2. What does std::call_once do?
//    - It executes a function only once for a given flag.

// 3. What header is needed for once_flag and call_once?
//    - <mutex>

// 4. Is std::call_once thread-safe?
//    - Yes, it is guaranteed to be thread-safe.

// 5. What happens if multiple threads call call_once?
//    - Only one will execute the function; others will wait or skip.


---------------------------------------------------------
   bind, forward, function, initializer_list, placeholders, invoke
---------------------------------------------------------

// std::bind:
// - Creates a callable object by binding arguments to a function.
// - Useful for partial function application and callbacks.

// std::forward:
// - Perfectly forwards arguments (preserves lvalue/rvalue nature).
// - Used inside templates to pass parameters efficiently.

// std::function:
// - Type-erased callable wrapper for functions, lambdas, bind, etc.
// - Enables storing and passing around callable entities.

// std::initializer_list:
// - Allows uniform initialization syntax using `{}`.
// - Commonly used in constructors and utility functions.

// std::placeholders:
// - Used with std::bind to specify placeholder positions (_1, _2, etc).

// std::invoke:
// - Introduced in C++17 (in <functional>).
// - Uniformly calls any callable: functions, lambdas, member functions, functors.
// - Especially useful in template and generic programming.

Questions:

// 1. What is std::bind used for?
//    - To bind arguments and create callable objects.

// 2. Why is std::forward important in templates?
//    - It preserves the value category (lvalue/rvalue) of parameters.

// 3. What is std::function?
//    - A general-purpose polymorphic function wrapper.

// 4. Where is std::initializer_list commonly used?
//    - In constructors for uniform list-style initialization.

// 5. What is std::invoke?
//    - A generic utility to call functions, lambdas, or member functions uniformly.


---------------------------------------------------------
   Types of Initialization in C++
---------------------------------------------------------

// 1. Copy Initialization → T x = 10;
// 2. Direct/Constructor Initialization → T x(10);
// 3. Uniform (Brace) Initialization → T x{10}; // prevents narrowing
// 4. Value Initialization → T(); // initializes to zero or default
// 5. List Initialization → T x = {1, 2, 3}; // uses initializer_list if available

Questions:

// 1. What is uniform initialization?
//    - Initialization using braces {} introduced in C++11.

// 2. How is copy initialization different from direct initialization?
//    - Copy may allow conversions; direct is stricter.

// 3. What is value initialization?
//    - It zero-initializes fundamental types and calls default constructor for class types.

// 4. When is initializer_list used?
//    - When a constructor with std::initializer_list is available.

// 5. Which form of initialization prevents narrowing?
//    - Uniform (brace) initialization.



---------------------------------------------------------
   Lambda Functions in C++
---------------------------------------------------------

// Lambdas are anonymous functions introduced in C++11.
// Useful for short, inline function definitions, especially for callbacks and STL algorithms.

// Syntax:
// [capture](parameters) -> return_type { body }; or [capture](parameters) mutable -> return_type { body };
// - Capture: [=] for by-value, [&] for by-reference


Questions:

// 1. What is a lambda in C++?
//    - An anonymous inline function used for local short tasks.

// 2. What does the capture clause in lambda do?
//    - Captures external variables by value or reference.

// 3. Can lambdas be stored in std::function?
//    - Yes, if they are copyable and match the signature.

// 4. How to return from a lambda?
//    - Use `return` inside the body, or omit it if using expression form.

---------------------------------------------------------
   chrono Library in C++ (Summary Only)
---------------------------------------------------------

// <chrono> is a C++ standard library for time utilities (since C++11).
// It provides types for durations, clocks, and timestamps.

// - steady_clock:
//   • Monotonic, not affected by system clock changes.
//   • Best for measuring elapsed time.

// - system_clock:
//   • Represents real-world time (can change).
//   • Used for logs, timestamps, file times.

// - high_resolution_clock:
//   • Provides the highest precision.
//   • Often aliases either steady or system clock (platform-dependent).

Questions:

// 1. What is <chrono> used for in C++?
//    - Measuring durations, intervals, and timestamps.

// 2. Which clock is best for measuring elapsed time?
//    - steady_clock.

// 3. What is the difference between system_clock and steady_clock?
//    - system_clock follows real-world time, steady_clock is monotonic.

// 4. Is high_resolution_clock always better than steady_clock?
//    - Not always; it may alias steady_clock internally.

// 5. Why use duration_cast in chrono?
//    - To convert time units (e.g., seconds to milliseconds).


---------------------------------------------------------
   Variadic Templates in C++
---------------------------------------------------------

// Introduced in C++11, variadic templates allow functions or classes to accept an arbitrary number of template arguments.
// Useful for forwarding, logging, tuple construction, and metaprogramming.

// Use Cases:
// - printf-like logging
// - Argument forwarding
// - Type-safe parameter packs
// - Compile-time operations using fold expressions (C++17+)

// Fold Expressions (C++17):
// - Allows concise expansion of parameter packs.
// - Syntax: (... op args) or (args op ...)
// - Example: template<typename... Args> auto sum(Args... args) { return (... + args); }

Questions:

// 1. What are variadic templates?
//    - Templates that accept zero or more template arguments.

// 2. What is a parameter pack?
//    - A pack that holds multiple template arguments.

// 3. How do you expand a parameter pack?
//    - Using recursion or fold expressions (C++17+).

// 4. Give one real-world use of variadic templates.
//    - Logging functions with arbitrary number of arguments.

// 5. When were variadic templates introduced?
//    - In C++11.

**********************************************/

#include <iostream>
#include <functional>
#include <utility>
#include <initializer_list>
#include <chrono>
#include <thread>

void greet(const std::string& name) {
	std::cout << "Hello, " << name << "";
}

void print_all(std::initializer_list<int> nums) {
	for (int n : nums) std::cout << n << " ";
	std::cout << "";
}

void bind_forward_demo() {
	using namespace std::placeholders;

	// std::bind with placeholder
	auto greet_user = std::bind(greet, _1);
	greet_user("Siva");

	// std::function
	std::function<void(std::string)> func = greet;
	func("ChatGPT");

	// std::initializer_list
	print_all({1, 2, 3, 4, 5});

	// std::forward demo
	auto forward_demo = [](auto&& arg) {
		greet(std::forward<decltype(arg)>(arg));
	};
	std::string name = "World";
	forward_demo(name);
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
