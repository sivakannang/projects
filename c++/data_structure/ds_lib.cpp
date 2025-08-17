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
	
	queue.pop();
	std::cout << queue.front() << std::endl;

	queue.push("three");
	std::cout << queue.front() << std::endl;
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
	
	dsa::HashTable<std::string, std::string> hash_table;
	
	hash_table.insert("BSE", "Bombay Stock Exchange");
	hash_table.insert("NSE", "National Stock Exchange");
	hash_table.insert("MCX", "Multi Commodity Exchange");
	hash_table.print();

	hash_table.remove("MCX");
	hash_table.print();

	auto optResult = hash_table.find("NSE");
	if ( optResult ) {
		std::cout << "find(NSE) = " << *optResult << std::endl;
	} else {
		std::cout << "find(NSE) = nullptr" << std::endl;
	}
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

	dsa::String s1;                     // default constructor
	dsa::String s2("siva");             // argument constructor
	dsa::String s3("kannan");           // argument constructor
	dsa::String s4(s2);                 // copy constructor
	dsa::String s5 = s3;                // copy constructor
	dsa::String s6 = s2 + s3;           // operator + and copy constructor
	

	std::cout << "s1 : " << s1 << std::endl;
	std::cout << "s2 : " << s2 << std::endl;
	std::cout << "s3 : " << s3 << std::endl;
	std::cout << "s4 : " << s4 << std::endl;
	std::cout << "s5 : " << s5 << std::endl;
	std::cout << "s6 : " << s6 << std::endl;
	
	s6 = s2 + s3 + s6;                  // operator + and copy assignment
	std::cout << "s6 : " << s6 << std::endl;
	s3 = s2 + s3 + s2 + s3;
	std::cout << "s3 : " << s3 << std::endl;

	if ( s3 == s6 )
	{
		std::cout << "s3 and s6 are same " << std::endl;
	}
	
	if ( !(s3 == s2) )
	{
		std::cout << "s3 and s2 are not same " << std::endl;
	}
	
	if ( s3 == s3 )
	{
		std::cout << "s3 and s3 are same " << std::endl;
	}

	std::cout << s3[0] << std::endl;  // const char& operator[] (size_t idx) const
	s3[0] = 'I';                      //       char& operator[] (size_t idx)
	std::cout << s3 << std::endl;

	dsa::String s7 = std::move(dsa::String("test"));  // std::move will create rvalue reference, move construtor will invoke
	std::cout << "s7 " << s7 << std::endl;
	s7 = std::move(dsa::String("done with the test"));              // std::move will create rvalue reference, move assginment will invoke
	std::cout << "s7 " << s7 << std::endl;
	
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
	graph_test();
	return 0;

}
