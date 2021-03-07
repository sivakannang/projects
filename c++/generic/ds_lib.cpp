/***********************************************
 *
 *  valgrind --tool=memcheck --leak-check=yes ./a.out
 *
 *
 * ******************************************/

#include <ds_lib.h>

void array_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
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

	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
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
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
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
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
	dsa::Queue<std::string> queue;

	queue.push("zero");
	queue.push("one");
	queue.push("two");

	std::cout << queue.front() << std::endl;
	std::cout << queue.back() << std::endl;
	
	queue.pop();
	std::cout << queue.front() << std::endl;
	std::cout << queue.back() << std::endl;

	queue.push("three");
	std::cout << queue.front() << std::endl;
	std::cout << queue.back() << std::endl;
}

void list_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
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
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
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

	std::cout << hash_table.find("five") << std::endl;

	// custom class as key required 2 additional functions
	// 1. std::equal_to<T> collision manage -> required to implement 'operator ==' overload function
	// 2. std::hash<T>     template specialization for std::hash struct for handle hash for custom class
	
	dsa::Hash<Employee, 10> emp_hash_table;
	emp_hash_table.insert(Employee(1, "siva", 100.00));
	emp_hash_table.insert(Employee(2, "kanna", 200.00));
	emp_hash_table.insert(Employee(3, "kannan", 300.00));
}

void unordered_map_test() {
	
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
	dsa::Unordered_map<int, std::string> unordered_map;

	unordered_map.insert(5, "five");
	unordered_map.insert(1, "one");
	unordered_map.insert(8, "eight");
	unordered_map.insert(3, "three");
	unordered_map.insert(4, "four");
	unordered_map.insert(4, "four");
	unordered_map.insert(11, "eleven");
	unordered_map.insert(12, "tweleve");
	unordered_map.insert(13, "thirteen");
	unordered_map.insert(14, "fourteen");
	unordered_map.insert(15, "fifteen");
	unordered_map.insert(16, "sixteen");

	unordered_map.traverse();
	std::cout << unordered_map.find(16) << std::endl;
	unordered_map.erase(16);
	unordered_map.traverse();

}

void map_test() {
	
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
	dsa::Map<int, std::string> map;

	map.insert(5, "five");
	map.insert(1, "one");
	map.insert(8, "eight");
	map.insert(3, "three");
	map.insert(4, "four");
	map.insert(4, "four");

	map.dfs_in_order();

}

void string_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;
	
	dsa::String s1 = "Test string";
	dsa::String s2 = "Sample String";
	std::cout << s1 << std::endl;
	std::cout << s2 << std::endl;
	
	dsa::String s3 = s2;
	std::cout << s3 << std::endl;
	
	dsa::String s4 = s1 + s2;
	std::cout << s4 << std::endl;
	
	s1 = s2;
	
	dsa::String s5 = std::move( dsa::String("sivakannan"));
	std::cout << s5 << std::endl;
	
	s5 = std::move(dsa::String("Kavikuyil"));
	dsa::String s6 = dsa::String("...........Siva"); // compiler optimize it only argument constructor called, non other construct calls here
	
	std::cout << s1 << std::endl;
	std::cout << s2 << std::endl;
	std::cout << s3 << std::endl;
	std::cout << s4 << std::endl;
	std::cout << s5 << std::endl;
	std::cout << s6 << std::endl;
}

void graph_test() {

	{	
		using namespace dsa::directed_graph;

		std::vector<Edge<int>> edges = { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 2, 1 }, { 3, 2 }, { 4, 5 }, { 5, 4 } };

		Graph<int> graph(edges, 6);
		graph.print();
	}

	{
		using namespace dsa::directed_graph_weight;

		std::vector<Edge> edges = {
			 // `(x, y, w)` —> edge from `x` to `y` having weight `w`
			 { 0, 1, 6 }, { 1, 2, 7 }, { 2, 0, 5 }, { 2, 1, 4 },
			 { 3, 2, 10 }, { 5, 4, 1 }, { 4, 5, 3 }
		};
		
		Graph graph(edges, 6);
		graph.print();
	}
}

int main()
{
	array_test();
	vector_test();
	stack_test();
	queue_test();
	list_test();
	hash_table_test();
	map_test();
	string_test();
	unordered_map_test();
	graph_test();

}
