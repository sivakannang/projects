/******************************************************************************************************************************************
 *
 * 
 *------------------------------------------------ Variable initialization ---------------------------------------------------------------
 *
 *      int x=1;   // C like initialization
 *      int y(2);  // Constructor initialization
 *      int z{3};  // Uniform initialization
 *----------------------------------------------------------------------------------------------------------------------------------------

 *------------------------------------------------- Default Arguments --------------------------------------------------------------------
 *
 * C++ introduces so-called default arguments for functions.
 * These default values would be used if the caller omits the corresponding actual argument in calling the function.
 * Default arguments are specified in the function prototype, and cannot be repeated in the function definition.
 * The default arguments are resolved based on their positions. Hence, they can only be used to substitute the trailing arguments to avoid ambiguity
 *
 * int fun1(int a = 1, int b = 2, int c = 3)
 * int fun2(int a,   int b = 2, int c = 3)
 * 
 * ---------------------------------------------------------------------------------------------------------------------------------------
 

 *------------------------------------------------- Function overloading -----------------------------------------------------------------
 *
 * C++ allows you to specify more than one definition for a function name or an operator in the same scope, which is called function overloading
 * and operator overloading respectively.
 * The definition of the function must differ from each other by the types and/or the number of arguments in the argument list.
 * You cannot overload function declarations that differ only by return type
 *
 * void fun(int, int, int);  // Version 1
 * void fun(double, int);    // Version 2
 * void fun(int, double);    // Version 3
 *
 * Overloaded functions cannot be differentiated by the return-type (compilation error)
 *
 * Name Mangling:
 * To differentiate bw different versions of overloaded function, many compilers(such as GNU GCC) adopt a name mangling or name decoration scheme for naming functions
 *
 * g++ function-overload.cpp
 *
 * List the symbol table     -> nm function-overload.o
 * ......
 * 000000b5 T __Z3fundi
 * 000000ed T __Z3funid
 * 00000089 T __Z3funiii
 * ......
 *
 *
 * Each of the function is identified via a prefix __Z
 * followed by an integer containing the number of characters of the function name (3 in this case for "fun")
 * followed by the parameter type list (where i for int and d for double).
 * For example, di means a double followed by an int; id for an int followed by a double; iii for 3 ints.
 *
 * ---------------------------------------------------------------------------------------------------------------------------------------


 *------------------------------------------------- inline vs macro ----------------------------------------------------------------------
 *
 *
 * Macro  -> Expanded by the preprocessor
 * inline -> parsed by the compiler
 *           Inline function can access the data members, where macro's can't
 *           Type checking is there in inline , where macro's doesn't has
 *           Debugging is easy, can put a debug pointer because it is expanding in the compilation time, where in macros we can't
 *           Compilers are free to decide, inline or function even thouhg we mentioned inline, so inline is a recomendation and __forceinline or attribute((always_inline)) is a strong recomendation
 *
 * #define SQUARE(x) x*x     // Macro with argument
 * inline int square(int x) { return x*x; }  // inline function
 *
 * cout << SQUARE(5+5) << endl;   // expand to 5+5*5+5 - wrong answer
 * cout << square(5+5) << endl;   // Okay square(10)
 *
 * int x = 2, y = 3
 * SQUARE(++x) << endl;   // expand to ++x*++x (16) - x increment twice
 * cout << square(++y) << endl;   // Okay ++y, (y*y) (16)
 *
 * Note :
 * * Inline function is preferred over macro expansion, as seen from the above example.
 * * Functions written inside class will always considered as inline functions, so write big functions outside class for reduce the code size
 *
 * ---------------------------------------------------------------------------------------------------------------------------------------
 


 * --------------------------------------------------- keyword this ---------------------------------------------------------------------
 *
 * this is a keyword that refers to the current instance of the class, it can be used
 *   - to pass current object as a parameter to another method
 *   - to refer current class instance variable
 *   - to declare indexers
 *   - to access the global member variable, when the function has an argument with same name as global name
 * many overloaded operators return *this
 *
 * Note:
 *  this keyword can be used only within member functions, not in global or static functions
 *  the reason is these static methods are called without using object, hence this pointer not initialized for this
 *  delete this -> If the object is created using new, then we can do delete this, otherwise behavior is undefined.
 *  
 *  In OOPS each object gets its own copy of data members and all objects share single copy of member functions, Then now question is that if only one copy of each member function exists and is used by multiple objects, how are the proper data members are accessed and updated?
 *    - Compiler supplies, ‘this’ pointer as a hidden argument to all nonstatic member function calls and is available as a local variable within the body of all nonstatic functions
 *
 * When a reference to a local object is returned, the returned reference can be used to chain function calls on a single object.
 *
 *   Class Test {

	 private:
		 int x, y;
	 public:
		 Test(int i = 0, int j = 0) { this->x = i, this->y = j ;  }
		 Test setX ( int a )        { x = a; return *this ; }
		 Test setY ( int b )        { y = b; return *this ; }
		 Test& setXp ( int a )        { x = a; return *this ; }
		 Test& setYp ( int b )        { y = b; return *this ; }
		 void print ( )             { cout << " x = " << x << " , y = " << y << endl; }
     }

     int main() {

	     Test test;
	     test.setX(10).setY(20).setX(30);
	     test.print();  // prints 10, 0  ( setting y will happen in temporary object)

	     Test tst;
	     tst.setXp(10).setYp(20);
	     tst.print();  // prints 10, 20
     }

 *  -------------------------------------------------------------------------------------------------------------------------------------
 

 *---------------------------------------------------- keyword constant -----------------------------------------------------------------
 *
 * Constant is something that doesn't change. In C and C++ it can be used with:
 *
 * Variables
 * Pointers
 * Function arguments and return types
 * Class Data members
 * Class Member functions
 * Objects
 * mutable with constant objects
 *
 * const var:
 *  Must be initialized during declaration
 *  Values can't be changed during runtime
 *  naming convention is uppercase with underscore, eg MIN_VALUE, MAX_SIZE
 *
 * const pointer:
 *
 *
 *
 * Function arguments and return types:
 *  const parameter/argument can't be changed inside function
 *  return type const doesn't effect in built-in data type(int , char , ...) but it prevents modifying the return object, if it is class
 *  If a function has a non-const parameter, it cannot be passed a const argument while making a call.
 *  
 *
 * const member in class:
 *  Should be initialized in constructor, else compiler throw error
 *  Once initialized the value can't be changed
	
	class Test {
		private:
			const int x;
		public:
			Test(int i = 0) : x(i) {}  // if we don't initialize const variable in constructor, compiler will throw error
			
			int getValue() const {
				return x;
			}
	};
 *
 *
 *
 *
 * const objects:
 *
 *  * When an object is declared or created using the const keyword, its data members can never be changed, during the object's lifetime. 
 *  * Temporary objects created while program execution are always of const type
 *
 *
 *
 * const member functions:
 *
 *  1. Constant member function not allow to change values of it's object/class members
 *  2. Non-const functions can only be called by non-const objects
 *  
 *  ex 1:
 *
 *  	class Test { 
    		int value; 
		public: 
    		Test(int v = 0) {value = v;} 
    		int getValue() const 
		{
			// We get compiler error if we add a line like "value = 100;" 
			return value;
		}
	        int increment(){
			return ++value;
		}	
	}

    ex 2:
        
        int main() { 
    		const Test t; 
    		cout << t.increment();   // compiler error - Non-const functions can only be called by non-const objects
    		return 0; 
	}

 *  
 *
 * mutable with const objects:
 *   mutable keyword is used with member variables of class, which we want to change even if the object is of const type.
 *   mutable is particularly useful if most of the members should be constant but a few need to be updateable.
 *   cannot use the mutable specifier with names declared as static or const, or reference
 *
 *   Consider this example where use of mutable can be useful.
 *   Suppose you go to hotel and you give the order to waiter to bring some food dish.
 *   After giving order, you suddenly decide to change the order of food.
 *   Assume that hotel provides facility to change the ordered food and again take the order of new food within 10 minutes after giving the 1st order.
 *   After 10 minutes order can’t be cancelled and old order can’t be replaced by new order.
 *   See the following code for details.
 *
	class Customer 
	{ 
    		char name[25];
		mutable char placedorder[50];
		int tableno;
		mutable int bill; 
	public:
    		Customer(char* s, char* m, int a, int p)
		{
			strcpy(name, s);
			strcpy(placedorder, m);
			tableno = a;
			bill = p;
		}
		
		void changePlacedOrder(char* p) const {
			strcpy(placedorder, p);
		}
		
		void changeBill(int s) const {
			bill = s;
		}
		
		void display() const { }
	};
  
	int main() 
	{ 
    		const Customer c1("Pravasi Meet", "Ice Cream", 3, 100); 
		c1.display();
		c1.changePlacedOrder("GulabJammuns");
		c1.changeBill(150);
		c1.display();
		return 0;
	}
 *   
 *
 * ---------------------------------------------------------------------------------------------------------------------------------------
 

 *----------------------------------------------------- keyword static -------------------------------------------------------------------
 *
 *  C static with variable -> variable visibility will be local to file, allocated storage only once in a program lifetime in static storage area
 *  C static with function -> method visibility local to the file
 *
 *  C++ static member variable :
 *    * no matter how many objects of the class are created, there is only one copy of the static member shared across all the objects
 *    * it must be initialized explicitly, always outside the class. If not initialized, Linker will give error
 *
 *  C++ static object          :
 *    * like variables, objects also when declared as static have a scope till the lifetime of program.
 *
 *  C++ static member function :
 *    * Static member functions are allowed to access only the static data members and other static member functions, else compiler throws error
 *    * It doesn't have any "this" keyword which is the reason it cannot access ordinary members
 *    * It can be called using an object and the direct member access . operator. But recommend to call using class name and scope resolution :: operator
 *
 *
 *  class Box {
	  
	 private:
		 int length, width;
	 public:
		 static int objectCount;
	 	 
		 Box(int a = 0, int b = 0) { cout << "Constructor" << endl; length(a), width(b); objectCount++; }
		 ~Box() { cout << "Destructor" << endl; objectCount--; }

		 static int getObjectCount() { return objectCount; }

		 int area() { return length * width; }
 
    };

    int Box::objectCount = 0;

    int main() {
	    cout << Box::getCount() << endl ;  // output will be zero
	    Box b1(2, 3);
	    Box b2;
	    cout << Box::getCount() << endl ;  // output will be 2

	    {
		    static Box b3;
	    }
	    cout << "Program exits" << endl;

Output:   -> As the static scope is till program ends, destructor will be called after program exit
	    Constructor
	    Program exits
	    Destructor

    }
 *
 * ---------------------------------------------------------------------------------------------------------------------------------------
 *
 *
 *----------------------------------------------------- class and object -----------------------------------------------------------------
 *
 * - struct vs class - by default class members are private, where the struct members are public. default inheritance of base class is private and struct is public
 * - Each object gets its own copy of the data member
 * - Each object shares the member functions, means only one member function exists in the code segment or memory
 *
 * * Constructor
 * * Destructor
 * * copy constructor, copy assignment
 * * move constructor, move assignment
 * * constructor and destructor calling order, explicit call of constructor and destructor
 * * Operator overloading
 * * Rule of 3 or Rule of 5
 *
 * C++ provide below default special member functions, which are used to create, destroy, initialize, convert, move, and copy class objects
 * Each class can select explicitly which of these members exist with their default definition or which are deleted by using the keywords default and delete
 *
 *      Member function                                               implicitly defined                                             default definition
 *
	Default constructor	C::C();                     if no other constructors                                                 does nothing
	Destructor		C::~C();                    if no destructor	                                                     does nothing
	Copy constructor	C::C (const C&);            if no move constructor and no move assignment	                     copies all members
	Copy assignment		C& operator= (const C&);    if no move constructor and no move assignment	                     copies all members
	Move constructor	C::C (C&&);                 if no destructor, no copy constructor and no copy nor move assignment    moves all members
	Move assignment		C& operator= (C&&);         if no destructor, no copy constructor and no copy nor move assignment    moves all members

	i.stack.imgur.com/b2VBV.png

 *
 * Constructor :
 *  - is a special method which use to initialize the object
 *  - but it differ from method in that it has no explicit return type
 *  - method name will be same as class name
 *  - C++ allows more than one constructor by overloading
 *  - every class will have a default constructor, until explicitly a constructor defined. In default constructor members will be initialized with value 0.
 *  - is usually declared in the public section, but can also be declared in the protected and private sections, if the user wants to restrict access to them
 *  - Constructor functions are not inherited and their addresses cannot be referenced
 *  - When memory allocation is required, the new and delete operators are called implicitly
 *
 *  class X{
	 private:
		 int a;
	 public:
		 X() {}         -> Don't define explicitly default constructor because compiler will implement the default constructor which is more efficient
		 X() =default;  -> Forcing a default constructor to be generated by the compiler
		 X() =delete    -> Avoiding implicit default constructor

    }
 *
 * Constructor has two parts :
 *  Initializer list:
 *   - Appears before method body, starts with a colon and entries are comma-separated.
 *   - Assignments occur according to the order in which data members are declared (even if the order in the initializer list is different).
 *   - There are situations where initialization of data members inside constructor doesn't work and Initializer List must be used
 *     * When no default constructor is present in the Base class
 *     * When reference type is used        -> References are immutable they can be initialized only once
 *     * For initializing const data member -> const data memebers can be initialized only once, so it must be initialized in the initialization list
 *     * When data member and parameter have same name -> if not vaue will be assigned as 0
 *     * For improving performance   -> Creation and destroy of temporary object can be avoided by using initializer list, by not assigning inside constructor body
 *  Body:
 *  - The second part is the body, which is a normal method body enclosed in curly brackets
 *
 * Destructor:
 *  - is a special class function which destroys the object as soon as the scope of object ends.
 *  - is called automatically by the compiler when the object goes out of scope
 *
 class Rectangle {
	private:
		int length;
		int width;
		const int x;
		int &y;
	public:
		~Rectangle(){};
		Rectangle(int length, int width, int x = 5, int y = 10) : length(length), width(width), x(x), y(y) {}
		int area() { return length * width ; }
};

class Box : public Rectangle {
	private:
		int height;
	public:
		Box(int length, int width, int height) : Rectangle(length, width), height(height) { }
		int volume() { return area() * height; }
};

 * Unnamed / temporary objects:  when move constructor will be invoked ?
 *  - Unnamed objects are objects that are temporary in nature, and thus haven't even been given a name.
 *  - Typical examples of unnamed objects are return values of functions or type-casts.
 *  - In these cases, there is no need to make a copy, because the unnamed object is very short-lived and can be acquired by the other object
 *
 *  shallow copy in constructor -> Shallow copy copies references to original objects
 *  deep copy in constructor    -> Shallow copy doesn't suit for the classes use pointers, where memories need to be allocated and then values need to be copied
 *  General requirements for deep copy
 *   - A normal constructor
 *   - A destructor to delete the dynamically allocated memory
 *   - A copy constructor to make a copy of the dynamically allocated memory
 *   - An overloaded assignment operator
 *
 *
 * copy constructor -> is used to create a copy of an already existing same type of object.
 * copy assignment  ->
 * move constructor ->
 * move assignment  ->
 *
 * MyClass foo;             // default constructor
 * MyClass bar (foo);       // object initialization: copy constructor called
 * MyClass baz = foo;       // object initialization: copy constructor called
 * foo = bar;               // object already initialized: copy assignment called
 *
 * MyClass fn();            // function returning a MyClass object
 * MyClass baz = fn();      // object initialization, unnamed/temp object: move constructor called
 * baz = MyClass();         // object already initialized: unnamed/temp object: move assignment called 
 *
 *
 *
 * rule of 3 or 5 : it claims that if one of these had to be defined by the programmer, then the compiler-generated version may not fit to the remaining these
 *
 * destructor
 * copy constructor
 * move constructor
 * copy assignment operator
 * move assignment operator 
 * ---------------------------------------------------------------------------------------------------------------------------------------

 * ***************************************************************************************************************************************/


#include <iostream>
#include <iomanip>
#include "cls-object.h"

using namespace std;

void testTime();
void testDefaultFunctions();


class Rectangle {

	private:
		string *ptr;
	public:
		
		const string& content() const { return *ptr;}
		void print() { cout << *ptr << endl; }

		Rectangle() : ptr(new string)
		{
			cout << "default constructor" << endl;
		}

		Rectangle(const string& ptr) : ptr(new string(ptr))
		{
			cout << "parameterized constructor" << endl;
		}

		Rectangle(const Rectangle& rect) : ptr(new string(rect.content()))
		{
			cout << "copy constructor" << endl;
		}
		

		Rectangle& operator= (const Rectangle& rect)
		{
			cout << "copy assignment" << endl;

			if ( &rect == this ) {
				cout << "self assignment" << endl;
				return *this;
			}

			delete ptr;
			ptr = new string(rect.content()) ;
			return *this;
		}

		Rectangle(Rectangle&& rect) : ptr(rect.ptr)  
		{
			cout << "move constructor" << endl;
			rect.ptr = nullptr;
		}

		Rectangle& operator= (Rectangle&& rect)
		{
			cout << "move assignment" << endl;

			if ( &rect == this ) {
				cout << "self assignment" << endl;
				return *this;
			}

			delete ptr;
			ptr = rect.ptr ;
			rect.ptr = nullptr;
			return *this;
		}

		Rectangle operator+ ( const Rectangle& rect)
		{
			return Rectangle(content() + rect.content());
		}

		~Rectangle()
		{
			if ( ptr != nullptr) {
				cout <<"destructor : " << *ptr << endl;
				delete ptr;
				ptr = nullptr;
			}else{
				cout <<"destructor : nullptr" << endl;
			}
		}



};

int main()
{
	testTime();
	short smallNumber = 1234567890;
	testDefaultFunctions();
	return 0;
}

void testDefaultFunctions() {

	Rectangle rect1;                     // default constructor
	Rectangle rect2("rect2");            // parameterised constructor
	Rectangle rect3(rect2);              // copy constructor
	rect3 = rect1;                       // copy assignment
	Rectangle rect4(std::move(Rectangle("rect4"))); // move constructor


	rect1.print();
	rect2.print();
	rect3.print();
	rect4.print();
	rect4 = Rectangle("rect5");           // move assignment
	rect4.print();

}


Time::Time(int h, int m, int s)
{
	setTime(h, m, s);
}

void Time::setHour(int h)
{
	if ( h >=0 && h <= 23)
		hour = h;
	else
		throw invalid_argument("Invalid hour! Hour shall be 0-23.");
}

void Time::setMin(int m)
{
	if ( m >=0 && m <= 59)
		min = m;
	else
		throw invalid_argument("Invalid minute! Minute shall be 0-59.");
}

void Time::setSec(int s)
{
	if ( s >=0 && s <= 59)
		sec = s;
	else
		throw invalid_argument("Invalid second! Second shall be 0-59.");
}

int Time::getHour() const
{
	return hour;
}

int Time::getMin() const
{
	return min;
}

int Time::getSec() const
{
	return sec;
}

void Time::setTime(int h, int m, int s)
{
	setHour(h), setMin(m), setSec(s);
}

void Time::print() const
{
	cout << setfill('0');
	cout << setw(2) << hour << ":" << setw(2) << min << ":" << setw(2) << sec << endl;
}

Time& Time::addTimeInto(const Time& time)
{
	this->hour += time.hour;
	this->min  += time.min;
	this->sec  += time.sec;
	return *this;
}

Time Time::addTimeReturnNew(const Time& time) const
{
	return Time(hour+time.hour, min+time.min, sec+time.sec);
}


void testTime()
{
	try {
		Time t2(25, 0, 0);
		t2.print();
	}catch(invalid_argument&  ia){
		cout << "Invalid argument exception : " << ia.what() << endl;
		Time t1(21, 56, 59);
		t1.print();
	}
	catch(...){
		cout << "Default Exception" << endl;
	}


	// Ordinary object
	Time t1(1, 2, 3);
	t1.print();  // 01:02:03

	// Object pointer
	Time* ptrT1 = &t1;
	(*ptrT1).print(); // 01:02:03
	ptrT1->print();   // 01:02:03
	// anObjectPtr->member is the same as (*anObjectPtr).member

	// Object reference
	Time& refT1 = t1; // refT1 is an alias to t1
	refT1.print();    // 01:02:03

	// Dynamic allocation
	Time* ptrT2 = new Time(4, 5, 6); // allocate dynamically
	ptrT2->print(); // 04:05:06
	delete ptrT2;   // deallocate

	// Object Array
	Time tArray1[2];    // tArray1 is an array of Time with 2 elements
	// Use default constructor for all elements
	tArray1[0].print();  // 00:00:00
	tArray1[1].print();  // 00:00:00

	Time tArray2[2] = {Time(7, 8, 9), Time(10)}; // Invoke constructor
	tArray2[0].print();  // 07:08:09
	tArray2[1].print();  // 10:00:00

	Time* ptrTArray3 = new Time[2]; // ptrTArray3 is a pointer to Time
	// Dynamically allocate an array of Time with 2 elements via new[]
	ptrTArray3[0].print();  // 00:00:00
	ptrTArray3[1].print();  // 00:00:00
	delete[] ptrTArray3;    // Deallocate dynamic array via delete[]

	// C++11 syntax, compile with -std=c++0x
	Time* ptrTArray4 = new Time[2] {Time(11, 12, 13), Time(14)}; // Invoke constructor
	ptrTArray4->print();        // 11:12:13
	(ptrTArray4 + 1)->print();  // 14:00:00
	delete[] ptrTArray4;

	Time t4(1,1,1);
	Time t5(2,2,2);
	t4.print();
	t5.print();
	t4.addTimeInto(t5);
	t4.print();
	t4.addTimeInto(t4).addTimeInto(t5).print();
	Time t6 = t4.addTimeReturnNew(t5);
	t4.print();
	t6.print();

}


