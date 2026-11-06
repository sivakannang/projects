/***********************************************
 *
 *  g++ -std=c++17 ds_lib.cpp -o ds_lib
 *
 *  Check for memory errors with either of these:
 *    g++ -std=c++17 -g -fsanitize=address,undefined ds_lib.cpp -o ds_lib && ./ds_lib
 *    valgrind --tool=memcheck --leak-check=yes ./ds_lib
 *
 * ******************************************/

#include "ds_lib.h"

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

	// Strings too long for the small string optimization, as those own heap memory, and show up memory bugs
	v.push_back("a string long enough to be stored on the heap");
	v.push_back(v[0]);       // An element of the vector itself, while it may be growing

	dsa::Vector<std::string> copy = v;
	copy.pop_back();
	std::cout << "v.size() = " << v.size() << ", copy.size() = " << copy.size() << std::endl;

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

	std::cout << list.front() << " ... " << list.back() << " , size " << list.size() << std::endl;
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
	dsa::String s6 = s2 + s3;           // operator +, with the result built directly in s6 (copy elision)


	std::cout << "s1 : " << s1 << std::endl;
	std::cout << "s2 : " << s2 << std::endl;
	std::cout << "s3 : " << s3 << std::endl;
	std::cout << "s4 : " << s4 << std::endl;
	std::cout << "s5 : " << s5 << std::endl;
	std::cout << "s6 : " << s6 << std::endl;
	
	s6 = s2 + s3 + s6;                  // operator + and move assignment, as the result is a temporary
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

	dsa::String temp("test");
	dsa::String s7 = std::move(temp);                  // Move constructor. temp is a named variable, so it needs std::move.
	std::cout << "s7 " << s7 << std::endl;
	s7 = dsa::String("done with the test");            // A temporary is already an rvalue, so move assignment runs without std::move
	std::cout << "s7 " << s7 << std::endl;
}

void mystring_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;

	dsa::MyString s1("siva");
	dsa::MyString s2(s1);                 // copy constructor
	dsa::MyString s3(std::move(s1));      // move constructor
	std::cout << "s1 size after move : " << s1.size() << std::endl;
	s1 = s2 + s3;                         // operator + and move assignment
	std::cout << s1 << " " << s2 << " " << s3 << std::endl;
}

void trie_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;

	dsa::Trie trie;
	for ( const char* word : { "car", "card", "care", "dog" } )
		trie.insert(word);

	std::cout << std::boolalpha << trie.search("car") << " " << trie.search("ca") << " " << trie.startsWith("ca") << std::endl;

	for ( const auto& word : trie.autoSuggest("car", 2) )
		std::cout << word << " ";
	std::cout << std::endl;
}

void priority_queue_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;

	dsa::PriorityQueue<int> max_heap;
	dsa::PriorityQueue<int, std::greater<int>> min_heap;

	for ( int x : { 5, 1, 9, 3, 7 } ) {
		max_heap.push(x);
		min_heap.push(x);
	}

	while ( !max_heap.empty() ) {
		std::cout << max_heap.top() << "/" << min_heap.top() << " ";
		max_heap.pop();
		min_heap.pop();
	}
	std::cout << std::endl;
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
			 // `(x, y, w)` -> edge from `x` to `y` having weight `w`
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
	mystring_test();
	trie_test();
	priority_queue_test();
	graph_test();
	return 0;
}
