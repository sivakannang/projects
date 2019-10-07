/*******************************************************************************
* exception hierarchy
* exception key words try, catch, throw, noexcept
* std::exception std::exception_ptr, std::current_exception(), std::rethrow_exception(), std::make_exception_ptr()
* exception hierarchy order change behaviour
* exception on threads
* exception inheritance and override
*
*
*/
#include <iostream>
#include <string>
#include <exception>



void exception_test_1() {
	try {
		std::string().at(1);
	}
	catch (std::exception& e) {
		std::cout << "e.what() -> " << e.what() << std::endl;
	}
}

void handle_eptr(std::exception_ptr e_ptr) {

	if (e_ptr) {
		try {
			std::rethrow_exception(e_ptr);
		}
		catch (std::exception& e) {
			std::cout << "e.what() -> " << e.what() << std::endl;
		}
	}
}

void exception_test_2() {
	std::exception_ptr e_ptr;
	try {
		std::string().at(1);
	}
	catch (...) {
		e_ptr = std::current_exception();
		handle_eptr(e_ptr);
	}
}

int main() {
	exception_test_2();
	return 0;
}
