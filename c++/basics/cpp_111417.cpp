/**************************************************** chrono library - time library ******************************************************
 *
 *
 * clocks:
 *  system_clock                 - wall clock time from the system-wide realtime clock
 *  steady_clock                 - monotonic clock that will never be adjusted
 *  high_resolution_clock        - the clock with the shortest tick period available
 *
 * Time Point:
 *  time_point                   - a point in time
 *
 * Duration:
 *  duration                     - a time interval
 *  duration_cast
 *
 * ex : chrono_library_test()
 *
 * ************************************************************* constexpr **************************************************************
 *
 *   - computation /object initialization at compile time, for better performence
 *   - conditional compilation by replacing the macros #ifdef
 *   - simplifying templates
 *
 *   ex : test_constexpr()
 *
 * ************************************************************* lampda *****************************************************************
 *
 * ex : test_lampda()
 * **************************************************************************************************************************************/


#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>

constexpr int fact(int i);
constexpr int fibanacci(int n); 
void generate_random(); 

void test_chrono_library(); 
void test_constexpr(int argc);

template<typename T>
void test_initializer_list(const std::initializer_list<T>& il);

void test_auto_var();


int main(int argc, char *args[], char **env) {

	//test_chrono_library();
	test_constexpr(argc);
	test_initializer_list({1, 2, 3, 4, 5, 6});
	test_initializer_list({"one", "two", "three", "four"});

	return 0;
}

constexpr int fact(int n) {
	
	if ( n == 0 )
		return 1;
	return n * fact(n-1);
}

constexpr int fibanacci(int n) {
	
	return  (n <= 1) ? 1 : fibanacci(n-1) + fibanacci(n-2);
}

void generate_random() {

	std::vector<int> v(25000);

	auto f = []() -> int { return rand() % 10000; };

	std::generate(v.begin(), v.end(), f);
	std::sort(v.begin(), v.end());
}

void test_chrono_library() {


	std::cout << "generate and sort vector(10000) begins" << std::endl;

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	//std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
	//std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	
	generate_random();

	auto end = std::chrono::system_clock::now();
	//auto end = std::chrono::steady_clock::now();
	//auto end = std::chrono::high_resolution_clock::now();
	
	std::cout << "generate and sort vector(10000) ends" << std::endl;

	std::cout << "execution time in nano  seconds : " << (end-start).count() << std::endl;
	std::cout << "execution time in nano  seconds : " << (std::chrono::duration_cast<std::chrono::nanoseconds>(end-start)).count() << std::endl;
	std::cout << "execution time in micro seconds : " << (std::chrono::duration_cast<std::chrono::microseconds>(end-start)).count() << std::endl;
	std::cout << "execution time in milli seconds : " << (std::chrono::duration_cast<std::chrono::milliseconds>(end-start)).count() << std::endl;
	std::cout << "execution time in       seconds : " << (std::chrono::duration_cast<std::chrono::seconds>(end-start)).count() << std::endl;

	std::cout << "execution time in nano  seconds : " << (std::chrono::duration<double, std::nano>(end-start)).count() << std::endl;
	std::cout << "execution time in micro seconds : " << (std::chrono::duration<double, std::micro>(end-start)).count() << std::endl;
	std::cout << "execution time in milli seconds : " << (std::chrono::duration<double, std::milli>(end-start)).count() << std::endl;
	std::cout << "execution time in milli seconds : " << (std::chrono::duration<double, std::ratio<1,1000>>(end-start)).count() << std::endl;
	std::cout << "execution time in       seconds : " << (std::chrono::duration<double, std::ratio<1,1>>(end-start)).count() << std::endl;
	std::cout << "execution time in       minutes : " << (std::chrono::duration<double, std::ratio<60*1,1>>(end-start)).count() << std::endl;
	std::cout << "execution time in       hours   : " << (std::chrono::duration<double, std::ratio<60*60*1,1>>(end-start)).count() << std::endl;
}

class Rectangle {
		int _h, _w;
		public:
			// A constexpr constructor
			constexpr Rectangle (int h, int w) : _h(h), _w(w) {}
			constexpr int getArea () const  {   return _h * _w; } 
};

void test_constexpr(int argc) {

	// set breakpoint at function 'fact'.
	const int val = fact(5);          // computed at compile time, fat function won't invoke here
	const int temp = fact(argc);      // computes at run time     ,because rhs argument is not const
	int tmp = fact(6);                // computes at run time    , because lhs is not const
	const int num = 4;
	const int v = fact(num);          // computed at compile time, fact function won't invoke here
	
	// Below object is initialized at compile time
	constexpr Rectangle rect(10, 20);
	std::cout << rect.getArea() << std::endl;

}

template<typename T>
void test_initializer_list(const std::initializer_list<T>& il) {

	std::for_each(il.begin(), il.end(), [](auto &d) {
			std::cout << d << std::endl;
			});
}


void test_auto_var() {
	std::map<int, std::string> map;
	
	map.insert({1, "one"});
	if ( map.insert({1, "one"}).second == false )
		std::cout << "insertion failed, duplicate not allowed" << std::endl;

	// this capture supports from C++17
	const auto& [it, status] = map.insert({2, "two"});
	std::cout << "insertion status : " << status << std::endl;

	for ( const auto& [key, val] : map ) 
		std::cout << key << " -> " << val << std::endl;
}

template<typename... Args>
void print(Args... args)
{
	(std::cout << ... << args) << std::endl;  // Fold expression C++17
	int sum = args + ...;
}

