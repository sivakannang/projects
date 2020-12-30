
#include <iostream>
#include <memory>

class A {
	private:
		int a;
		int *p;
	public:
		A(int a = 0, int *ip = nullptr) : a(a) , p(new int) { *p = ip ? *ip : 0 ; }
		A(const A& obj)
		{
			p = new int;
			a = obj.a;
			*p = *(obj.p);
		}
		A& operator = (const A& obj)
		{
			//delete p;
			//p = new int;
			a = obj.a;
			*p = *(obj.p);
			return *this;
		}
		A operator + (const A& obj)
		{
			A tmp;
			tmp.a = a + obj.a;
			*(tmp.p) = *p + *(obj.p);
			return tmp;
		}
		~A() { std::cout << __func__ << " " << a << std::endl; delete p; }

		friend std::istream& operator >> (std::istream& istr, A& obj)
		{
			std::cout << "Enter a = ";
			istr >> obj.a;
			std::cout << std::endl << "Enter *p : ";
			istr >> *(obj.p);
			std::cout << std::endl;
			return istr;
		}

		friend std::ostream& operator << (std::ostream& ostr, const A& obj)
		{
			ostr << obj.a << " " << *(obj.p) ;
			return ostr;
		}
};

int test_1()
{
	A obj1, obj2, obj3;
	A obj('t');

	obj3 = obj1 + obj2;
	obj3 = obj2;

	A obj4 = obj1;
	A obj5(10);

	std::cin >> obj1;
	std::cout << obj1;

	return 0;
}

int test_2()
{
	int eleven = 11, twelve = 12, thirteen = 13, fourteen = 14, fifteen = 15;

	A obj1(1, &eleven), obj2(2, &twelve), obj3(3, &thirteen), obj4(4, &fourteen), obj5(5, &fifteen);

	std::cout << "obj1 -> " << obj1 << std::endl; // 1 11
	std::cout << "obj2 -> " << obj2 << std::endl; // 2 12
	std::cout << "obj3 -> " << obj3 << std::endl; // 3 13
	std::cout << "obj4 -> " << obj4 << std::endl; // 4 14
	std::cout << "obj5 -> " << obj5 << std::endl; // 5 15

	obj3 = obj1 + obj2 + obj3 + obj4 + obj5;      // ~A(3) , ~A(6), ~A(10) ~A(15)
	std::cout << "obj3 -> " << obj3 << std::endl; // 15 65

	A obj6 = obj3;
	std::cout << "obj6 -> " << obj6 << std::endl; // 15 65

	return 0;

/***************************** output **********************

obj1 -> 1 11
obj2 -> 2 12
obj3 -> 3 13
obj4 -> 4 14
obj5 -> 5 15
~A 15
~A 10
~A 6
~A 3
obj3 -> 15 65
obj6 -> 15 65
~A 15
~A 5
~A 4
~A 15
~A 2
~A 1
***********************************************************/
}

int main()
{
	return test_2();
}
