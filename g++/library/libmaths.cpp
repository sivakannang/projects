
#include <iostream>
#include "libmaths.h"


int average(int a, int b, int c) {
	return (a+b+c)/3;
}



Maths::Maths(int a, int b, int c) : _a(a), _b(b), _c(c) { std::cout << "Constructor invoked" << std::endl; }

int Maths::avg() { return (_a + _b + _c)/3; }




void* maths_create(int a, int b, int c){
	std::cout << "Instance creating" << std::endl;
	Maths *p = new Maths(a, b, c);
	std::cout << "Instance created" << std::endl;
	return p;
}

int maths_avg(void *p) {
	std::cout << "Instance method invoked" << std::endl;
	int result = static_cast<Maths*>(p)->avg();
	std::cout << "Instance method completed" << std::endl;
	return result;
}

void maths_destroy(void *p) {
	std::cout << std::endl << "Instance deleting" << std::endl;
	delete static_cast<Maths*>(p);
	std::cout << "Instance deleted" << std::endl;
}



