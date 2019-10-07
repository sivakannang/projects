/* ******************************************************************* casting *************************************************************
 *
 * upcast    -> derived to base
 * downcast  -> base to derived
 *
 *
 * C++ provides 4 types of explicit type castings
 *
 * 	- static_cast          : static_cast<new_type>(expression)
 * 	- dynamic_cast         : dynamic_cast<new_type>(expression)
 * 	- const_cast           : const_cast<new_type>(expression)
 * 	- reinterpret_cast     : reinterpret_cast<new_type>(expression)
 *
 * 	-- volatile and typeid
 *
 *
 *
 * static_cast:
 *
 * 	- can perform conversions between pointers to related classes, not only from the derived class to its base, but also from a base class to its derived. 
 * 	- numerical conversions
 * 	- can't cast two unrelated classes
 * 	- no safety check performed ( ex : base class to derived class casting always works but it could lead undefined behaviour at runtime )
 * 
 *
 *
 * dynamic_cast:
 *
 * 	- used to casts from one pointer or reference of a polymorphic type to another, similar to static_cast but performing a type safety check
 * 	  at runtime to ensure the validity of the cast( ex : base class to derived class cast fails)
 * 	- casting allowed only in pointer or reference.
 * 	- attempt to cast to a pointer type, and that type is not an actual type of the argument object, then the result of the cast will be NULL.
 * 	- attempt to cast to a reference type, and that type is not an actual type of the argument object, then the cast will throw a std::bad_cast exception
 *
 * 	TYPE& dynamic_cast<TYPE&> (object)
 * 	TYPE* dynamic_cast<TYPE*> (object)
 *
 *
 * const_cast:
 *
 * 	- this type of casting manipulates the constness of an object, either to be set or to be removed
 * 	- target and argument type should be same, else compliation error will occur
 * 	- const_cast can also be used to cast away volatile attribute
 * 	- trying to modify a, by default declared as constant variable, behaviour is undefined
 *
 * 	ie :- You are not allowed to const_cast variables that are actually declared as const. This results in undefined behavior.
 * 	      const_cast is used to remove the const-ness from references and pointers that ultimately refer to something that is not const
 *
 *
 *
 * reinterpret_cast:
 *
 * 	- converts any pointer type to any other pointer type, even of unrelated classes
 * 	- reinterpret_cast treats all pointers exactly as traditional type-casting operators do.
 *
 *
 * typeid:
 * 	- ANSI-C++ also defines a new operator called typeid that allows to check the type of an expression
 * 	- header <typeinfo>
 * 	- ex
 *
 *
 * volatile:
 * 	- informing to the compiler , avoid optimization for the specific variable
 *
 * ****************************************************************************************************************************************/

#include <iostream>
#include <typeinfo>
//#include <thread>
//#include <unistd.h>
//#include <signal.h>
//#include <atomic>

using namespace std;


class CSubstraction {
    float _i, _j;
	public:
    CSubstraction(float i = 20.0, float j= 10.0) : _i(i), _j(j) {}
    void setter(float i , float j)                {_i = i, _j = j;}
    float result() { cout << "sub result() "; return _i - _j;}
};

class CAddition {
    float _i, _j;
  public:
    CAddition(float i = 20.0, float j= 15.0) : _i(i), _j(j) {}
    void setter(float i , float j)            {_i = i, _j = j;}
    float result() { cout << "add result() "; return _i + _j;}
};


void test_static_cast();
void test_dynamic_cast();
void test_const_cast(const CSubstraction &sub);
void test_reinterpret_cast();
void test_typeid();

void chk_null(int i, void *p);
void test_volatile();


int main () {

	test_static_cast();

	CSubstraction sub(30.00, 15.00);
	test_const_cast(sub);
	test_dynamic_cast();
	test_typeid();

	test_volatile();

	return 0;
  
}

void test_static_cast() {

	CSubstraction sub;
	CAddition *add;
	void *p;

	// traditional c style casting
	// object add, calls it's function result() but use object sub's member's, which can cause a crash or unexpected behaviour
	add = (CAddition*) &sub;
	cout << add->result();  // prints 30 , instead of 35

	//add = static_cast<CAddition*>(&sub); // not ok, unrelated class casting. static_cast avoid's this and produces compilation error due to type check
	add = static_cast<CAddition*>(p);      // ok. void * to class pointer
}

void test_const_cast(const CSubstraction &sub) {

	int i = 3;                 // i is not declared const
	const int& rci = i; 
	const_cast<int&>(rci) = 4; // OK: modifies i
	std::cout << "i = " << i << '\n';

	const int j = 3; // j is declared const
	int* pj = const_cast<int*>(&j);
	// *pj = 4;      // bad, undefined behavior

	//cout << "sub() : " << sub.result() << endl;  // not ok, Non-const functions can only be called by non-const objects
	//sub.setter(31.00, 10);                       // not ok, Non-const functions can only be called by non-const objects
	CSubstraction &sub_tmp = const_cast<CSubstraction&>(sub);
	sub_tmp.setter(31.00, 10);                            // not ok, const object, can't be modifiled
	cout << "sub() : " << sub_tmp.result() << endl;
}

void test_dynamic_cast() {

	struct A {
		virtual void f() { }
	};
	struct B : public A { };
	struct C { };


	A a;
	B b;

	try {
		A* ap = &b;
		B* b1 = dynamic_cast<B*> (&a);  // NULL, because 'a' is not a 'B', base to derived always fails
		chk_null(1, b1);
		B* b2 = dynamic_cast<B*> (ap);  // NON-NULL because 'b' to 'b'
		chk_null(2, b2);
		C* c = dynamic_cast<C*> (ap);   // NULL, because 'B' is not 'C', fails because cast between unrelated classes
		chk_null(3, c);
		A *aa = dynamic_cast<A*>(&b);   // NON-NULL because 'b' to 'a', derived to base always pass
		chk_null(4, aa);


		A& ar = dynamic_cast<A&> (*ap); // Ok, derived to base
		B& br = dynamic_cast<B&> (*ap); // Ok, cast between same type class/object
		C& cr = dynamic_cast<C&> (*ap); // std::bad_cast , fails because cast between unrelated classes
	}catch( exception &e){
		cout << "exception detail : " << e.what() << endl;
	}

}

void chk_null(int i, void *p)
{
	cout << i;
	if ( p == NULL)
		cout << " nullptr";
	else
		cout << "non-null";
	cout << endl;
}


void test_reinterpret_cast() {

	class A {};
	class B {};
	A * a = new A;
	B * b = reinterpret_cast<B*>(a);
}

void test_typeid() {

	CSubstraction s1, *s2, *s3;
	CAddition a1, *a2;

	cout << typeid(s1).name() << endl;
	cout << typeid(s2).name() << endl;
	cout << typeid(a1).name() << endl;
	cout << typeid(a2).name() << endl;

	if ( typeid(s1) == typeid(s2) )
		cout << "s1 and s2 types are same" << endl;
	else
		cout << "s1 and s2 types are different" << endl;  // it prints this bcoz S1 is CSubstraction and S2 is CSubstraction*

	if ( typeid(s3) == typeid(s2) )
		cout << "s2 and s3 types are same" << endl; // it prints this bcoz both S3 and S2 is CSubstraction* 
	else
		cout << "s2 and s3 types are different" << endl;


	struct A { virtual ~A() { } };
	struct B : A { };

	struct C { };
	struct D : C { };

	B bobj;
	A* ap = &bobj;
	A& ar = bobj;
	cout << "ap: " << typeid(*ap).name() << endl;
	cout << "ar: " << typeid(ar).name() << endl;

	D dobj;
	C* cp = &dobj;
	C& cr = dobj;
	cout << "cp: " << typeid(*cp).name() << endl;
	cout << "cr: " << typeid(cr).name() << endl;

	/*
	output:
	    
	    ap: B
	    ar: B
	    cp: C
	    cr: C

	    Classes A and B are polymorphic; classes C and D are not.
	    Although cp and cr refer to an object of type D, typeid(*cp) and typeid(cr) return objects that represent class C.
	*/
}
/*
class Gadget
{
	private:
		volatile bool _flag;

	public:
		//void wait()
		void operator() ()
		{
			while (!_flag)
			{
				print(1);
				sleep(1); // sleeps for 1000 milliseconds
			}
		}
		
		void wakeup() {
			_flag = true;
			cout << "woke up.." << _flag << endl;
			print(2);
		}

		void print(int i) { cout << i << " : flag : " << _flag << endl; }
	
	
};
int quit;

void sig_handler(int signal) {
	cout << "signal received : SIGINT" << endl;
	quit = 1;
}

void run() {

	quit = 0;
	while ( !quit ) { // compiler can optimize this something like while ( true ) or only first time it will read quit value from memory and cache it, then it reads from the cache everytime, it will skip to read from memory
		cout << "app is running" << endl;
		sleep(1);
	}
}


void test_volatile() {

	Gadget g1;

	std::thread t(g1);
	sleep(10);
	g1.wakeup();
	t.join();
	cout << "exiting" << endl;

	//signal(SIGINT, sig_handler);

	//run();

	
	cout << "app exists" << endl;

}
*/
