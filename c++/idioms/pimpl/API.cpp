
#include <iostream>
#include <memory>
#include <API.h>

class API::Impl {
	private:
		int a;
		int b;
};

API::API() : impl_(std::make_unique<Impl>()) {}
API::~API() = default;
void API::execute() { std::cout << "executed from library, added a & b" << std::endl; }
