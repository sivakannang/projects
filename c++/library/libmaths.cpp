
#include <iostream>
#include "libmaths.h"


int average(int a, int b, int c) {
	return (a+b+c)/3;
}


Maths::Maths(int a, int b, int c) : _a(a), _b(b), _c(c) { std::cout << "Constructor invoked" << std::endl; }

int Maths::avg() const { return (_a + _b + _c)/3; }


// C code can't handle an exception, so nothing may be thrown across this boundary
MathsHandle* maths_create(int a, int b, int c) {
	try {
		std::cout << "Instance creating" << std::endl;
		Maths *p = new Maths(a, b, c);
		std::cout << "Instance created" << std::endl;
		return reinterpret_cast<MathsHandle*>(p);
	} catch (...) {
		return nullptr;
	}
}

int maths_avg(const MathsHandle *p) {
	std::cout << "Instance method invoked" << std::endl;
	int result = reinterpret_cast<const Maths*>(p)->avg();
	std::cout << "Instance method completed" << std::endl;
	return result;
}

void maths_destroy(MathsHandle *p) {
	std::cout << "Instance deleting" << std::endl;
	delete reinterpret_cast<Maths*>(p);
	std::cout << "Instance deleted" << std::endl;
}
