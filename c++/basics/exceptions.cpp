
/* ---------------------------------------------------------
   Basics of Exception Handling in C++
---------------------------------------------------------

// - C++ uses try, catch, and throw keywords for exception handling.
// - Allows separating error handling code from regular code.
// - Exceptions are objects; can be of any type (usually std::exception or custom types).
// - Catch blocks must appear in order from most derived to base class.
// - If an exception is not caught, std::terminate() is called.


// - Stack unwinding: When an exception is thrown, local objects in the call stack are destroyed in reverse order (like popping the call stack).
// - noexcept: Introduced in C++11, specifies a function won’t throw exceptions.
//   • Enables compiler optimizations like removing exception-handling overhead.
//   • Required for some STL operations (e.g., move_if_noexcept in vector).
//   • Use `noexcept` to declare functions that are guaranteed not to throw.
//   • If a noexcept function throws, std::terminate is called.

// Exception Class Hierarchy (Standard Types):
//
// std::exception (base)
// ├── std::logic_error
// │   ├── std::invalid_argument
// │   ├── std::domain_error
// │   ├── std::length_error
// │   └── std::out_of_range
// └── std::runtime_error
//     ├── std::range_error
//     ├── std::overflow_error
//     ├── std::underflow_error
//     └── std::system_error
//
// - std::exception::what() returns an explanatory string.
// - It's recommended to use these standard types for clarity and debugging.
// Commonly Used Exception Utilities:
//
// - std::exception::what(): returns explanatory string for the exception.
// - std::current_exception(): captures current exception into std::exception_ptr.
// - std::rethrow_exception(): rethrows an exception held in std::exception_ptr.
// - std::terminate(): called when no handler is found or a noexcept function throws.

// Exception Safety Levels:
//
// 1. No-Throw Guarantee:
//    - Function is guaranteed not to throw. Declared using `noexcept`.
//
// 2. Strong Exception Guarantee:
//    - If an exception is thrown, the program state remains unchanged (like rollback).
//
// 3. Basic Exception Guarantee:
//    - The program remains in a valid state (no leaks), but state may be partially modified.
//
// 4. No Guarantee:
//    - No promises are made. Object may be left in an invalid state.

Questions:

// 1. What are the key keywords for exception handling?
//    - try, catch, and throw.

// 2. Can any type be thrown as an exception?
//    - Yes, but using classes derived from std::exception is preferred.

// 3. What happens if an exception is not caught?
//    - std::terminate() is called.

// 4. What is the advantage of using exception handling?
//    - Clean separation of error-handling logic from core logic.

// 5. What should be avoided in exception handling?
//    - Throwing from destructors, catching by value.

// 6. What is stack unwinding?
//    - Automatic destruction of local objects when an exception is thrown.

// 7. What is the use of noexcept?
//    - Declares that a function does not throw; helps with optimization.

// 8. What happens if a noexcept function throws?
//    - std::terminate() is called.


--------------------------------------------------------- */

#include <iostream>
#include <string>
#include <exception>
#include <memory>

void mayThrow(bool trigger) {
	if (trigger) {
		throw std::runtime_error("Something went wrong");
	}
}

void exception_demo() {
	try {
		mayThrow(true);
	} catch (const std::runtime_error& e) {
		std::cout << "Caught exception: " << e.what() << "\n";
	} catch (...) {
		std::cout << "Caught unknown exception\n";
	}
}




class MyCustomException : public std::exception {
	std::string message;
	public:
	MyCustomException(const std::string& msg) : message(msg) {}
	const char* what() const noexcept override {
		return message.c_str();
	}
};

void throw_custom_exception() {
	throw MyCustomException("Custom error occurred");
}



void rethrow_demo() {
	std::exception_ptr eptr;
	try {
		throw std::runtime_error("Captured via std::current_exception");
	} catch (...) {
		eptr = std::current_exception();
	}

	try {
		if (eptr) std::rethrow_exception(eptr);
	} catch (const std::exception& e) {
		std::cout << "Rethrown exception: " << e.what() << std::endl;
	}
}
int main() {
	exception_demo();
	try {
		throw_custom_exception();
	} catch (const MyCustomException& ex) {
		std::cout << "Caught custom exception: " << ex.what() << std::endl;
	}
	rethrow_demo();
}
