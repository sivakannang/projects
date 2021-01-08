/***********************************************
 *
 *  valgrind --tool=memcheck --leak-check=yes ./a.out
 *
 *
 * ******************************************/

#include <iostream>
#include <algorithm>
#include <string>
#include <ds_lib.h>

void array_test()
{
	dsa::Array<int, 4> arr;
	arr[0] = 0, arr[1] = 1, arr[2] = 2, arr[3] = 3;

	for ( auto a : arr )
		std::cout << a << std::endl;
	std::for_each(arr.begin(), arr.end(), [](auto i) { std::cout << i << std::endl; });
	
	dsa::Array<std::string, 4> arr1;
	arr1[0] = "kanna", arr1[1] = "ganesh", arr1[2] = "karthi", arr1[3] = "kavi";
	
	for ( auto a : arr1 )
		std::cout << a << std::endl;
	std::for_each(arr1.begin(), arr1.end(), [](auto &i) { std::cout << i << std::endl; });
}

void vector_test()
{

	dsa::Vector<std::string> v;

	v.push_back("siva");
	v.push_back("kannan");
	v.push_back("ganesh");
	v.push_back("karthi");
	v.push_back("Kavi");
	v.push_back("test");
	v.pop_back();
	
	for ( auto a : v )
		std::cout << a << std::endl;
	std::for_each(v.begin(), v.end(), [](auto &i) { std::cout << i << std::endl; });

}
int main()
{
	//array_test();
	vector_test();

}
