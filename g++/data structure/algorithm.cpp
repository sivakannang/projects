
/******************************************************** STL Algorithm ***************************************************************
 *
 * The header <algorithm> defines a collection of functions especially designed to be used on ranges of elements
 *
 ************************************************************************************************************************************/

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>

void print_array(auto &arr);
void test();

void non_modifying_sequence_ops_test();
void modifying_sequence_ops_test();
void partitions_test();

void min_max_test();
void sorting_test();
void merging_test();

void binary_search_test();
void heap_test();

void print_array(auto &arr){
	for ( auto &e : arr )
		std::cout << e << " ";
	std::cout << std::endl;
}

int main() {

	non_modifying_sequence_ops_test();
	//heap_test();

	return 0;
}


 /**************************************************** Algorithm - Non Modifying sequence operations **********************************
 *
 *  - all_of		-> Test condition on all elements in range
 *  - any_of		-> Test if any element in range fulfills condition
 *  - none_of		-> Test if no elements fulfill condition
 *  - for_each		-> Apply function to range
 *  - find		-> Find value in range
 *  - find_if		-> Find element in range
 *  - find_if_not	-> Find element in range
 *  - find_end		-> used to find the last occurrence of a sub-sequence inside a container
 *  - find_first_of	-> Used to compare elements between two containers. Returns iterator of first occurance of common element.
 *  - adjacent_find	-> first occurrence of two consecutive elements that match
 *  - count		-> Count appearances of value in range
 *  - count_if		-> Return number of elements in range satisfying condition
 *  - mismatch		-> Return first position where two ranges differ
 *  - equal		-> Test whether the elements in two ranges are equal
 *  - is_permutation	-> Test whether range is permutation of another
 *  - search		-> Search range for subsequence
 *  - search_n		-> Search range for elements
 *
 *
 *  bool all_of (InputIterator first, InputIterator last, UnaryPredicate pred)
 *  bool any_of (InputIterator first, InputIterator last, UnaryPredicate pred)
 *  bool none_of (InputIterator first, InputIterator last, UnaryPredicate pred)
 *
 *  Function for_each (InputIterator first, InputIterator last, Function fn)
 *
 *  InputIterator find (InputIterator first, InputIterator last, const T& val)
 *  InputIterator find_if (InputIterator first, InputIterator last, UnaryPredicate pred)
 *  InputIterator find_if_not (InputIterator first, InputIterator last, UnaryPredicate pred)
 *
 *  ForwardIterator1 find_end (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2)
 *  ForwardIterator1 find_end (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred)
 *
 *  ForwardIterator1 find_first_of (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2)
 *  ForwardIterator1 find_first_of (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred)
 *
 *  ForwardIterator adjacent_find (ForwardIterator first, ForwardIterator last)
 *  ForwardIterator adjacent_find (ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
 *
 *  count (InputIterator first, InputIterator last, const T& val)
 *  count_if (InputIterator first, InputIterator last, UnaryPredicate pred)
 *
 *  ForwardIterator1 search (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2)
 *  ForwardIterator1 search (ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred)
 *
 *  ForwardIterator search_n (ForwardIterator first, ForwardIterator last, Size count, const T& val)
 *  ForwardIterator search_n ( ForwardIterator first, ForwardIterator last, Size count, const T& val, BinaryPredicate pred )
 *
 *  bool equal (InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
 *  bool equal (InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicate pred)
 *
 *  pair<InputIterator1, InputIterator2> mismatch (InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
 *  pair<InputIterator1, InputIterator2> mismatch (InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicate pred)
 *
 *
 ***************************************************************************************************************************************/

struct Even {

	public:
		bool operator() (auto i) {
			return (i % 2 == 0);
		}
};

struct Odd {

	public:
		bool operator() (auto i) {
			return (i % 2 == 1);
		}
};

struct Print {
	public:
		bool operator() ( auto i) {
			std::cout << i << " ";
		}
};

struct Equal {
	public:
		bool operator() ( auto i, auto j) {
			return (i == j);
		}
};

void non_modifying_sequence_ops_test() {

	std::vector<int> vi = { 1, 3, 5, 7, 9, 11}; // initialize a vector elements with odd values
	print_array(vi);

	if ( std::all_of(vi.begin(), vi.end(), [](int i)  {return i % 2;}) ) {
		std::cout << "all are odd numbers" << std::endl;                     // it prints this line
	} else {
		std::cout << "not all are odd numbers" << std::endl;

	}

	Even even;
	Odd odd;
	std::cout << "any_of() -> Is any one even number ? " << std::boolalpha << std::any_of( vi.begin(), vi.end(), even) << std::endl; // prints false

	// increments the vector element values by 1, so that all values becomes even
	for ( auto &v : vi)
		v = v+1;
	print_array(vi);

	
	std::cout << "all_of() -> Is All even numbers    ? " << std::boolalpha << std::all_of( vi.begin(), vi.end(), even) << std::endl; // prints true
	
	std::cout << "none_of()-> Is None of odd numbers ? " << std::boolalpha << std::none_of( vi.begin(), vi.end(), odd) << std::endl; // prints true


	Print print;
	for_each( vi.begin(), vi.end(), print);
	std::cout << std::endl;

	std::cout << *(vi.end()) << vi[6] << std::endl;

	if ( std::find(vi.begin(), vi.end(), 6) == vi.end() ) {
		std::cout << "element not found" << std::endl;
	}

	auto it= std::find(vi.begin(), vi.end(), 6);
	if ( it == vi.end() ) {
		std::cout << "element not found" << std::endl;
	} else { 
		std::cout << "Element found : " << *it << std::endl;
	}

	// vector [ 2 4 6 8 10 12 ]
	if ( std::find_if(vi.begin(), vi.end(), odd) == vi.end() )
		std::cout << "No odd elements in the vector" << std::endl;  // it prints this line
	print_array(vi);

	it = std::find_if_not(vi.begin(), vi.end(), odd);  
	std::cout << "find_if_not odd returns at index : " << it - vi.begin() << " , and value is " << *it << std::endl;  // prints index 0, value 2

	vi.push_back(8);
	vi.push_back(10);
	vi.push_back(12);
	print_array(vi);
	// vector [ 2 4 6 8 10 12 8 10 12]
	std::vector<int> keys = { 8, 10, 12};
	
	it = std::find_end(vi.begin(), vi.end(), keys.begin(), keys.end());
	std::cout << "find_end range search returns at index : " << it - vi.begin() << " , and value is " << *it << std::endl;  // prints index 6, value 8

	it = std::find_first_of(vi.begin(), vi.end(), keys.begin(), keys.end());
	std::cout << "find_first_of range search returns at index : " << it - vi.begin() << " , and value is " << *it << std::endl;  // prints index 3, value 8

	keys[2] = 14;
	Equal equal;
	it = std::find_first_of(vi.begin(), vi.end(), keys.begin(), keys.end(), equal);
	// prints index 3, value 8, because it returns even any one element match in the given set, but find_end expects complete set
	std::cout << "find_first_of range search returns at index : " << it - vi.begin() << " , and value is " << *it << std::endl;


	vi.insert(vi.begin() + 3, 8);
	vi.insert(vi.begin() + 6, 12);
	print_array(vi); // [ 2 4 6 8 8 10 12 12 8 10 12 ]
	it = adjacent_find(vi.begin(), vi.end());
	std::cout << "adjacent_find() index : " << it - vi.begin() << " , and value is " << *it << std::endl;  // index 3, value 8
	it = adjacent_find(++it, vi.end(), equal);
	std::cout << "adjacent_find() index : " << it - vi.begin() << " , and value is " << *it << std::endl;  // index 6, value 12

	std::cout << "count(10) from vi     : " << std::count(vi.begin(), vi.end(), 10) << std::endl;          // prints 2
	std::cout << "count(even()) from vi : " << std::count_if(vi.begin(), vi.end(), even) << std::endl;     // prints 11


	keys[2] = 12; // keys = { 8, 10, 12}
	vi.insert(vi.begin() + 3, 8);
	print_array(vi); // [ 2 4 6 8 8 8 10 12 12 8 10 12 ]
	it = std::search(vi.begin(), vi.end(), keys.begin(), keys.end());
	std::cout << "search(8, 10, 12) from vi : found at index " << it - vi.begin() << " , value " << *it << std::endl;  // index 5, value 8
	it = std::search_n(vi.begin(), vi.end(), 3, 8);
	std::cout << "search_n(3, 8) from vi : found three 8's at index " << it - vi.begin() << " , value " << *it << std::endl; // index 3, value 8

	std::cout << "equal(vi, keys) = " << std::boolalpha << std::equal(vi.begin(), vi.end(), keys.begin()) << std::endl;        // prints false
	keys.clear();
	keys = vi;
	std::cout << "equal(vi, keys) = " << std::boolalpha << std::equal(vi.begin(), vi.end(), keys.begin(), equal) << std::endl; // prints true

	keys[5] = 9;
	print_array(vi);   // [ 2 4 6 8 8 8 10 12 12 8 10 12 ]
	print_array(keys); // [ 2 4 6 8 8 9 10 12 12 8 11 12 ]

	auto itr = std::mismatch(vi.begin(), vi.end(), keys.begin(), equal);
	// prints mismatch index 5 , value 8 9
	std::cout << "mismatch(vi, keys) : mismatch index " << itr.first - vi.begin() << " , value "  << *itr.first << " " << *itr.second << std::endl;
	std::cout << &vi[0] << " " << vi.data() <<  std::endl;
	++itr.first, ++itr.second;
	auto itr1 = std::mismatch(itr.first, vi.end(), itr.second, equal);
	std::cout << "mismatch(vi, keys) : mismatch index " << itr1.first - vi.begin() << " , value "  << *itr1.first << " " << *itr1.second << std::endl;

}

 /**************************************************** Algorithm - Modifying sequence operations **********************************
  * ***********************************************************************************************************************************/

 /* *************************************************** Algorithm - Heap ****************************************************************
 *
 *  - make_heap      -> Constructs a max heap in the range [first, last]
 *  - push_heap      -> Inserts the element at the position last-1 into the max heap defined by the range [first, last-1).
 *  - pop_heap       -> Swaps the value in the position first and the value in the position last-1
 *  - sort_heap      -> Converts the max heap [first, last) into a sorted range in ascending order. The resulting range no longer has the heap property.
 *  - bool is_heap   -> Checks if the elements in range [first, last) are a max heap. compared using operator< or given binary comparition function comp
 *  - is_heap_until  -> Examines the range [first, last) and finds the largest range beginning at first which is a max heap
 *
 *  definition:
 *  	- void make_heap( RandomIt first, RandomIt last )
 *	- void push_heap( RandomIt first, RandomIt last )
 *	- void pop_heap( RandomIt first, RandomIt last )
 *	- void sort_heap( RandomIt first, RandomIt last )
 *	- bool is_heap( RandomIt first, RandomIt last )
 *	- RandomIt is_heap_until( RandomIt first, RandomIt last )
 *
 *   example : heap_test()
 *   
 * *************************************************************************************************************************************/


void heap_test() {

	std::vector<int> v = { 10, 20, 30, 40, 50, 60, 70};
	print_array(v);  // -> 10 20 30 40 50 60 70

	std::make_heap(v.begin(), v.end() );
	print_array(v);  // -> 70 50 60 40 20 10 30

	v.push_back(80);
	print_array(v);  // -> 70 50 60 40 20 10 30 80

	std::push_heap(v.begin(), v.end());
	print_array(v);  // -> 80 70 60 50 20 10 30 40

	std::cout << "is_heap() : " << std::boolalpha << std::is_heap(v.begin(), v.end()) << std::endl;

	std::pop_heap(v.begin(), v.end());
	print_array(v);  // -> 70 50 60 40 20 10 30 80
	
	std::vector<int>::iterator it = std::is_heap_until(v.begin(), v.end());
	for ( auto i = v.begin(); i != it; i++)
		std::cout << *i << " ";
	std::cout << std::endl;
	
	v.pop_back();
	print_array(v);  //-> 70 50 60 40 20 10 30

	std::sort_heap(v.begin(), v.end());
	print_array(v);  //-> 10 20 30 40 50 60 70

	std::make_heap(v.begin(), v.end() );
	print_array(v);  // -> 70 50 60 40 20 10 30

}


