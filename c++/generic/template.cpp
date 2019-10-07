
#include <iostream>
#include <string>
#include <vector>
#include <set>

void printargs(int argc, char *argv[], char **env);

class Person {
	private:
		std::string firstname;
		std::string lastname;
		int number;
	public:
		Person(std::string fn, std::string ln, int num) : firstname(fn), lastname(ln), number(num) {}
		int getNumber() const { return number; }
};
//template function
template <class T>
T max(T t1, T t2) {
	return t1 > t2 ? t1 : t2;
}


//template class
template<class T>
class Accum {
	private:
		T total;
	public:
		Accum(T start) : total(start) {}
		T getTotal() const            { return total;}
		T operator+=(T const& t)      { return total = total + t ; }
};


//template specialization
template<>
class Accum<Person> {
	private:
		int total;
	public:
		Accum(int start) : total(start) {}
		int getTotal() const            { return total;}
		int operator+=(Person const& t)      { return total = total + t.getNumber() ; }
};

int main(int argc, char *argv[], char **env) {

	int a = 10, b = 20;

	printargs(argc, argv, env);

	std::cout << max(a, b) << std::endl;
	std::cout << max<double>(a, 20.0) << std::endl;

	Accum<int> integers(0);
	integers += 50;
	integers += 10;
	std::cout << integers.getTotal() << std::endl;

	Accum<std::string> strings("");
	strings += "Hello ";
	strings += "World";
	std::cout << strings.getTotal() << std::endl;

	Person p1("siva", "kannan", 10);
	Person p2("siva", "ganesh", 20);
	Person p3("siva", "karthi", 30);
	Accum<Person> people(0);
	people += p1;
	people += p2;
	people += p3;
	std::cout << people.getTotal() << std::endl;

	return 0;
}

void printargs(int argc, char *argv[], char **env){

	std::vector<std::string> v(argv, argv+argc);
	for ( auto it = v.begin(); it != v.end(); it++)
		std::cout << *it << std::endl;

	std::string str = "sivakannan";
	std::set<char> s(str.c_str(), str.c_str() + 10);
	for ( auto it = s.begin(); it != s.end(); it++)
		std::cout << *it << std::endl;
	std::cout << s.at(2) << std::endl;
}
