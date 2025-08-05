/************************ Operators **********************************
* 
* - Arithmetic: +, -, *, /, %
* - Assignment: =, +=, -=, *=, /=, %=
* - Increment/Decrement: ++, --
* - Comparison: ==, !=, <, >, <=, >=
* - Logical: &&, ||, !
* - Bitwise: &, |, ^, ~, <<, >>
* - Compound bitwise assignment: &=, |=, ^=, <<=, >>=
* - Ternary: condition ? expr1 : expr2
* - Scope resolution: ::
* - Member access: ., ->
* - Pointer: *, &
* - sizeof, typeid, alignof, decltype
* - new/delete, new[]/delete[]
* - dynamic_cast, static_cast, reinterpret_cast, const_cast

*---------------------  Interview Questions and Answers -------------------------------*
*
* 1. What is the difference between prefix and postfix increment?
*    - Prefix (++i) increments first, then returns; postfix (i++) returns first, then increments.
*
* 2. What is the difference between logical AND (&&) and bitwise AND (&)?
*    - Logical AND returns boolean; bitwise AND operates on bits.
*
* 3. How do operator precedence and associativity affect expressions?
*    - They determine the order in which operators are evaluated.
*
* 4. What is the use of the scope resolution operator (::)?
*    - To access global or class members when names are ambiguous.
*
* 5. What is the ternary operator and where is it useful?
*    - condition ? expr1 : expr2; it's a compact if-else expression.
*
* 6. How does dynamic_cast differ from static_cast?
*    - dynamic_cast does runtime checks, mainly for polymorphism.
*
* 7. What does sizeof return?
*    - The size in bytes of a type or variable at compile time.
*
* 8. What are the differences between & (address-of) and * (dereference)?
*    - & gets the memory address; * accesses the value at a pointer.
*
*---------------------------------------------------------------------------------*/    

#include <iostream>
#include <typeinfo>

class Base {
	public:
		virtual void show() { std::cout << "Base\n"; }
};

class Derived : public Base {
	public:
		void show() override { std::cout << "Derived\n"; }
};

class Int {
	int val;
	public:
	Int(int v) : val(v) {}

	// Pre-increment
	Int& operator++() {
		++val;
		return *this;
	}

	// Post-increment
	Int operator++(int) {
		Int temp = *this;
		++val;
		return temp;
	}

	void print() const {
		std::cout << val << std::endl;
	}
};

void operatorDemo() {
	int a = 10, b = 3;
	std::cout << "Arithmetic: " << a + b << ", " << a % b << std::endl;

	a += b;
	std::cout << "Assignment: a += b => " << a << std::endl;

	std::cout << "Comparison: (a == b)? " << (a == b) << std::endl;
	std::cout << "Logical: (a > 5 && b < 5)? " << ((a > 5 && b < 5) ? "true" : "false") << std::endl;

	std::cout << "Bitwise: a & b = " << (a & b) << std::endl;
	std::cout << "Ternary: " << ((a > b) ? "a > b" : "a <= b") << std::endl;

	int* p = &a;
	std::cout << "Pointer: *p = " << *p << ", &a = " << &a << std::endl;

	std::cout << "sizeof(a) = " << sizeof(a) << ", alignof(int) = " << alignof(int) << std::endl;

	// Pre/Post increment demo
	Int x(5);
	std::cout << "Pre-increment: ";
	(++x).print();

	Int y(5);
	std::cout << "Post-increment: ";
	(y++).print();
	std::cout << "After post-increment: ";
	y.print();
}

int main() {
	operatorDemo();
	return 0;
}

