#include "API.h"

#include <iostream>
#include <memory>

class API::Impl {
	public:
		Impl(int a, int b) : a_(a), b_(b) {}
		void set_a(int a) { a_ = a; }
		void execute() const { std::cout << a_ << " + " << b_ << " = " << a_ + b_ << std::endl; }
	private:
		int a_;
		int b_;
};

API::API(int a, int b) : impl_(std::make_unique<Impl>(a, b)) {}
API::~API() = default;

API::API(const API& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}
API& API::operator=(const API& other) { *this = API(other); return *this; }   // Copy, then move assign. Safe for self assignment.
API::API(API&& other) noexcept = default;
API& API::operator=(API&& other) noexcept = default;

void API::set_a(int a) { impl_->set_a(a); }
void API::execute() const { impl_->execute(); }
