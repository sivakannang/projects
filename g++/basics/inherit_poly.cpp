
/************************************************ Inheritance & Polymorphism *****************************************************************
 *
 * exception in constructor/destructor
 * friend
 * override
 *
 * Inheritance:
 * 	- Inheritance is the process by which objects of one class acquire the properties and actions of another class.
 * 	- This is possible by deriving a new class from the existing one, The new class will have combined features of both the classes. 
 *
 * Why inheritance ?
 *  - code reuse
 *  - code redability
 *
 * Access control in inheritance:
 *
 * 				Derived Class	Derived Class	Derived Class
 * 		Base class	Public Mode	Private Mode	Protected Mode
 * 		Private		Not Inherited	Not Inherited	Not Inherited
 * 		Protected	Protected	Private		Protected
 * 		Public		Public		Private		Protected
 *
 *
 * Types of inheritance in C++:
 *
 * 	- Single Inheritance
 * 	- Multiple Inheritance
 * 	- Hierarchical Inheritance
 * 	- Multilevel Inheritance
 * 	- Hybrid Inheritance (also known as Virtual Inheritance), it is a combination of 2 or more above inheritance
 *
 *  Single                           -> class sub   : public base_class
 *
 *  Multiple                         -> class sub   : public base_class_1, public base_class_2
 *
 *  Hierarchical                     -> class sub_1 : public base_class
 *  					class sub_2 : public base_class
 *  					class sub_3 : public base_class
 *
 *  Multi level                      -> class sub_1 : public base_class,
 *  					class sub_2 : public sub_1
 *
 *  Hybrid - virtual                 -> class sub_1 : public base_class
 *  					class sub_2 : public base_class
 *  					class sub_3 : public sub_1, public sub_2
 *
 *  Inheritance order : inheriting order ( class C : public A, public B , here class A will be called first and then class B)
 *  Constructor order : Base class to Derived class
 *  Destructor  order : Derived class to Base class
 *
 *  Inheritance, Diamond Problem or ambiguity, virtual inheritance   --->  https://www.geeksforgeeks.org/multiple-inheritance-in-c/
 *
 *  
 *  Functions that are never Inherited :
 *
 *   - Constructors and Destructors are never inherited and hence never overrided
 *   - Also, assignment operator = is never inherited. It can be overloaded but can't be inherited by sub class.
 *
 *  Inheritance and Static Functions:
 *
 *   - They are inherited into the derived class.
 *   - If you redefine a static member function in derived class, all the other overloaded functions in base class are hidden.
 *   - Static Member functions can never be virtual
 *
 * 
 *  Function Override:
 *   - It is the redefinition of base class function in its derived class with same signature i.e return type and parameters.
 *
 *
*
 *
 * **********************************************************  Polymorphism *******************************************************************
 *
 * Polymorphism is a feature of OOPs that allows the object to behave differently in different conditions. In C++ we have two types of polymorphism
 *
 * 1) Compile time Polymorphism ( static  or early binding , binding done by type of the pointer   ) , ex: Function overload, operator overloading
 * 2) Runtime Polymorphism      ( dynamic or late  binding , binding done by content of the pointer) , ex: Function override
 *
 *
 * Virtual functions:
 *
 * 	* A virtual function a member function which is declared within base class and is re-defined (Overriden) by derived class.
 * 	* When you refer to a derived class object using a pointer or a reference to the base class, you can call a virtual function for that object
 * 	  and execute the derived class’s version of the function.
 *
 *
 * Rules for virtual functions:
 *
 * 	* They Must be declared in public section of class.
 * 	* Virtual functions cannot be static and also cannot be a friend function of another class.
 * 	* Virtual functions should be accessed using pointer or reference of base class type to achieve run time polymorphism.
 * 	* The prototype of virtual functions should be same in base as well as derived class.
 * 	* They are always defined in base class and overridden in derived class. It is not mandatory for derived class to override
 * 	  (or re-define the virtual function), in that case base class version of function is used.
 * 	* A class may have virtual destructor but it cannot have a virtual constructor.
 *
 *
 * Mechanism of late binding : ( vPtr, vTable )
 *
 *      vPtr   -> a pointer to an vTable
 *      vTable -> a table that contains the function pointers of a class/object
 *
 * 	   - Every class that uses virtual functions (or is derived from a class that uses virtual functions) is given its own virtual table.
 * 	   - This table is simply a static array that the compiler creates at compile time.
 * 	   - A virtual table contains one entry for each virtual function that can be called by objects of the class.
 * 	   - Each entry in this vTable is simply a Function Pointer that points to the most-derived function accessible by that class ie the most Base Class.
 * 	   - The compiler also adds a hidden pointer to the base class, which we will call *__vPtr and it is inherited by derived class as well.
 * 	   - When a class object is created, *__vPtr is set to point to the virtual table for that class. 
 *
 * 	ex ref :
 * 	   - https://www.quora.com/What-are-vTable-and-VPTR-in-C++
 *
 *
 * Note:
 * 	* Using Virtual Keyword, we can access the private method of derived class. Compiler checks for access specifier only at compile time
 * 	  ex : https://www.studytonight.com/cpp/virtual-functions.php
 *
 * 	* Only the Base class Method's declaration needs the Virtual Keyword, not the definition.
 * 	* If a function is declared as virtual in the base class, it will be virtual in all its derived classes.
 * 	* The address of the virtual Function is placed in the VTABLE and the compiler uses VPTR(vpointer) to point to the Virtual Function.
 *
 *
 *
 * Pure Virtual Function:
 *
 * 	- virtual functions with no definition, they start with virtual keyword and ends with = 0. Here is the syntax for a pure virtual function,
 * 	- ex: virtual void f() = 0;
 * 	- We can give definition for pure virtual function, but should be given outside of class.
 * 	- Inline pure virtual definition is Illegal, compiler will give error
 *
 * 	class Base {						// Abstract Base class
		public:
			virtual void print() = 0;               // Pure virtual function
	};

	int main{
		Base *b;   // allowed
		Base b;    // compile time error
	}
 *
 *
 * 
 * Abstract class:
 *
 * 	- Abstract Class is a class which contains atleast one Pure Virtual function in it.
 * 	- Abstract classes are used to provide an Interface for its sub classes
 *
 * Characteristics of Abstract Class:
 *
 * 	- Abstract class cannot be instantiated, but pointers and refrences of Abstract class type can be created.
 * 	- Abstract class can have normal functions and variables along with a pure virtual function.
 * 	- Abstract classes are mainly used for Upcasting, so that its derived classes can use its interface.
 * 	- Classes inheriting an Abstract Class must implement all pure virtual functions, or else they will become Abstract too.
 *
 * Why can't we create Object of Abstract Class ?
 *
 *
 *
 * Virtual and Pure Virtual Destructors:
 *
 * 	- Destructors in the Base class can be Virtual.
 * 	- Whenever Upcasting is done, Destructors of the Base class must be made virtual for proper destruction of the object when the program exits.
 *
 * 	- Pure virtual destructors are legal in C++, there is no requirement of implementing pure virtual destructors in the derived classes
 *
 * 	- ref : https://www.studytonight.com/cpp/virtual-destructors.php
 *
 * 	note : Constructors are never Virtual, only Destructors can be Virtual.
 *
 *
 *
 * *************************************************************************************************************************************************/



#include <iostream>

using namespace std;

#ifdef _INHERITANCE_TEST_
class Shape {

	protected:
		int length, width;
	public:
		Shape(int l, int w) : length(l), width(w) {}
		int area() { return length * width ; }
};

class Rectangle : public Shape {
	public:
		//int length = 2; if we uncomment this, area() will use this variable instead of the base class variable
		Rectangle(int l, int w) : Shape(l, w) {}
		int area() { return length * width * 2 ;}
		int area_base() { return ( (Shape *) this)->area() ;}
};


int main() {

	Rectangle rect(4,7);
	cout << " area is " << rect.area() << endl;                          // prints 56
	cout << " area is " << rect.area_base() << endl;                     // prints 28
	cout << " area is " << ((Shape *)&rect)->area() << endl;             // prints 28
	cout << " area is " << static_cast<Shape&>(rect).area() << endl;     // prints 28
	//cout << " area is " << static_cast<Shape *>(rect)->area() << endl; // prints 28

	return 0;
}

#else

class Shape {
	protected:
		int _length, _width;
	public:
		Shape(int length = 0, int width = 0): _length(length), _width(width) {}
		virtual int area() { std::cout << "Base class area() is : " << std::endl;  return 0; }
		virtual ~Shape() = default; //when we have virutual method in the base class, then base class should define the destructor as virtual for proper destroy
};

class Rectangle : public Shape {
	public:
		Rectangle(int length = 0, int width = 0) : Shape(length, width) {}
		int area() { std::cout << "Rectangle class area() is : " << _length * _width << std::endl; return _length * _width; }
};

class Triangle : public Shape {
	public:
		Triangle(int length = 0, int width = 0) : Shape(length, width) {}
		int area() { std::cout << "Triangle class area() is : " << (_length* _width) / 2 << std::endl; return (_length * _width) / 2 ;}
};


int main(){

	Shape *shape;
	Rectangle rect(10, 5);
	Triangle  tri(10, 5);

	shape = &rect;
	shape->area();  // prints 50

	shape = &tri;
	shape->area();  // prints 25


	return 0;
}

#endif
