/***********************************************
 *
 *  valgrind --tool=memcheck --leak-check=yes ./a.out
 *
 *
 * ******************************************/

#include <ds_lib.h>

void array_test()
{
	dsa::Array<int, 4> arr;
	arr[0] = 0, arr[1] = 1, arr[2] = 2, arr[3] = 3;

	for ( auto a : arr )
		std::cout << a << std::endl;
	//std::for_each(arr.begin(), arr.end(), [](auto i) { std::cout << i << std::endl; });  // auto in lambda supports from c++14 only
	std::for_each(arr.begin(), arr.end(), [](int i) { std::cout << i << std::endl; });
	
	dsa::Array<std::string, 4> arr1;
	arr1[0] = "zero", arr1[1] = "one", arr1[2] = "two", arr1[3] = "three";
	
	for ( auto a : arr1 )
		std::cout << a << std::endl;
	std::for_each(arr1.begin(), arr1.end(), [](std::string& i) { std::cout << i << std::endl; });
}

void vector_test()
{

	dsa::Vector<std::string> v;

	v.push_back("zero");
	v.push_back("one");
	v.push_back("two");
	v.push_back("three");
	v.push_back("four");
	v.push_back("five");
	v.pop_back();
	
	for ( auto a : v )
		std::cout << a << std::endl;
	std::for_each(v.begin(), v.end(), [](std::string& i) { std::cout << i << std::endl; });

}

void stack_test()
{
	dsa::Stack<std::string> stack;

	stack.push("zero");
	stack.push("one");
	stack.push("two");

	std::cout << stack.top() << std::endl;
	
	stack.pop();
	std::cout << stack.top() << std::endl;

	stack.push("three");
	std::cout << stack.top() << std::endl;
}

void queue_test()
{
	dsa::Queue<std::string> queue;

	queue.push("zero");
	queue.push("one");
	queue.push("two");

	std::cout << queue.top() << std::endl;
	
	queue.pop();
	std::cout << queue.top() << std::endl;

	queue.push("three");
	std::cout << queue.top() << std::endl;
}

void list_test()
{
	dsa::List<std::string> list;

	list.push_back("zero");
	list.push_back("one");
	list.push_back("two");
	list.push_front("three");
	list.push_front("four");
	list.push_front("five");

	list.pop_back();
	list.pop_front();
}

void hash_table_test()
{
	dsa::Hash<std::string, 10> hash_table;
	
	hash_table.insert("zero");
	hash_table.insert("one");
	hash_table.insert("two");
	hash_table.insert("three");
	hash_table.insert("four");
	hash_table.insert("five");
	hash_table.insert("six");
	hash_table.insert("seven");
	hash_table.insert("eight");
	hash_table.insert("nine");
	hash_table.insert("ten");
	hash_table.insert("eleven");
	hash_table.insert("twelve");
	hash_table.traverse();

	hash_table.erase("two");
	hash_table.traverse();

	// custom class as key required 2 additional functions
	// 1. std::equal_to<T> collision manage -> required to implement 'operator ==' overload function
	// 2. std::hash<T>     template specialization for std::hash struct for handle hash for custom class
	
	dsa::Hash<Employee, 10> emp_hash_table;
	emp_hash_table.insert(Employee(1, "siva", 100.00));
	emp_hash_table.insert(Employee(2, "kanna", 200.00));
	emp_hash_table.insert(Employee(3, "kannan", 300.00));
}

int main()
{
	array_test();
	vector_test();
	stack_test();
	queue_test();
	list_test();
	hash_table_test();

}
