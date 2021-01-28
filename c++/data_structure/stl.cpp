
// g++ stl.cpp -std=c++11

/*****************************************************  STL - Standarad template Library ( Generic library ) ***************************
 *
 * It is a set of C++ template classes that provide generic classes and function that can be used to implement data structures and algorithms.
 * STL is mainly composed of :
 *
 * 	- Algorithms
 * 	- Containers
 * 	- Iterators
 *
 * Following are the some common containers
 *
 * 	- array               -> fixed array
 * 	- vector              -> dynamic growing array
 * 	- list                -> Doubly linked list
 * 	- forward_list        -> Single linked list
 * 	- deque               -> double ended queue
 * 	- stack               -> gives functionality of stack
 * 	- queue               -> gives functionality of queue
 * 	- priority queue      -> tree with max value as a root [ binary heap max tree, tree structure will be stored in array, by default vector ]
 * 	- map                 -> key value pair, sorted by key [ red black tree ( self balanced binary search tree) ]
 * 	- set                 -> unique keys   , sorted by key [ red black tree ( self balanced binary search tree) ]
 * 	- multimap, multiset  -> same as map and set but duplicates are allowed, sorted by key [ red black tree ( self balanced binary search tree) ]
 * 	- unordered{_map,_set,_multimap,_multiset} -> same like map, set, multimap, multiset but, not ordered/sorted [ Hash Table ]
 *
 * Classification of Containers:
 *
 * 	- Sequence containers               [ array, vector, list, forward_list, deque ]
 * 	- Associative containers            [ Ordered - map, set, multimap, multiset. Unordered - unordered_map, unordered_set, unordered_multimap, unordered_multiset]
 * 	- Containers adaptors               [ stack, queue, priority_queue ]
 *
 *
 *
 * must know questions :
 *
 * 	1. How these container works internally ?
 * 	2. What is the time and space complexity for these containers ?
 * 	3. When to use which containers ?
 *
 * Reference:
 * 	- https://en.cppreference.com/w/cpp/container/
 *
 * ********************************************************* pair and tuple ************************************************************
 *
 * header : utility
 *
 * syntax :
 * 	- pair<T1, T2> var                  ( pair can use it's member variabe pair.first and pair.second for getter/setter, it can also use std::get and std::tie )
 * 	- tuple<T1, T2, T3> var             ( tuple will use std::tie for getter and std::get for getter/setter )
 *
 * commonly used functions:
 *
 * 	- Operator = : assign values to a pair.
 * 	- swap : swaps the contents of the pair.
 * 	- Operators( == , != , > , < , <= , >= ) : lexicographically compares two pairs.
 * 	
 * 	- std::make_pair()  : create and returns a pair having objects defined by parameter list.
 * 	- std::make_tuple() : creates a tuple object of the type defined by the argument types
 * 	- std::tie(x, y, z)=tupleX : unpack the tupleX into individual objects 
 * 	- std::get<n>(std::tuple)  : tuple accesses nth specified element
 * 	- std::tuple_element : returns the type of tuple element
 * 	- std::tuple_size<std::tuple>::value    : returns number of elements in the tuple
 *
 * ex : pair_tuble_test()
 *
 *
 * ***************************************************** Iterator **********************************************************************
 *
 * Iterators play a critical role in connecting algorithm with containers along with the manipulation of data stored inside the containers.
 * 
 * An iterator is similar to pointer can point to elements in an array and can iterate through them using the increment operator (++).
 *
 * But, all iterators do not have similar functionality as that of pointers.
 *
 * Based upon the functionality they can be classified into five major categories:
 * 
 *
 * syntax : container_type <parameter_list>::iterator iterator_name
 *
 * operations in iterator:
 *
 * 	- begin()                                   - method returns an iterator to the start of the given container.
 * 	- end()                                     - method returns an iterator to the end of the given container.
 * 	- next(iterator i ,int n)                   - It will return the nth iterator to i, i.e iterator pointing to the nth element from the element pointed by i
 * 	- prev(iterator i ,int n)                   - It will return iterator pointing to the nth predecessor element from the element pointed by i.
 * 	- advance(iterator i ,int distance)         - It will increment/decrement the iterator i by the value of the distance
 * 	- distance(iterator first, iterator second) - It will return the number of elements or we can say distance between the first and the last iterator.
 *
 * 	advance and next/prev diffrence ?
 * 		advance()   - result will be written in the passed argument, doesn't return anything, bidirectional
 * 		next/prev() - result will be return in the new copy, passed argumnet will not be modified, one directional either forward/backward
 *
 *
 * Types of Iterators:
 *
 * 	1. Input Iterators ( only be used in a single-pass algorithms, no element is accessed more than once )
 * 	2. Output Iterators ( only be used in a single-pass algorithms, but not for accessing elements, but for being assigned elements )
 * 	3. Forward Iterator ( combined of input and output iterators, moves in forward direction and that too one step at a time )
 * 	4. Bidirectional Iterators ( same as forward iterator but can move both the directions )
 * 	5. Random-Access Iterators ( powerful iterator, combined all of the above iterator features, provides random access, this is the one whose functionality is similar to pointer )
 *
 * ex : iterator_test()
 *
 ******************************************************* Container functions **********************************************************

 * Element access :
 * 	at                              -> access specified element with bounds checking, throws out_of_range exception 
 * 	operator[]                      -> access specified element
 * 	front                           -> access the first element 
 * 	back                            -> access the last element 
 * 	data                            -> Returns a pointer to the first element of the object
 *
 * Iterators:
 * 	begin, cbegin                   -> returns an iterator/const iterator to the beginning 
 * 	end, cend                       -> returns an iterator/const iterator to the end
 * 	rbegin, crbegin                 -> returns a reverse iterator/const iterator to the beginning
 *	rend, crend                     -> returns a reverse iterator/const iterator to the end
 *
 *
 * Capacity:
 *	empty                           -> checks whether the container is empty
 *	size                            -> returns the number of elements
 *	max_size                        -> returns the maximum possible number of elements. size and max_size always safe
 *	reserve                         -> reserves least storage, so that vector expand and copy operation can be reduced
 *	capacity                        -> returns the number of elements that can be hold in currently allocated storage
 *	shrink_to_fit                   -> release the unused memory from the container, after this method call, size and capacity will be equal
 *
 * Modifiers:
 *	clear                           -> clear the container contents, this method call will make the size to 0, but capacity remains same
 *  assign                          -> Assigns new contents to the vector, replacing its current contents, and modifying its size accordingly
 *	insert                          -> insert an element in the beginning or before the iterator position. v.insert(5) or v.insert(v.begin()+2, 5)
 *	emplace(c++11)                  -> constructs element in-place, it helps to avoid temporary copy, use emplace in place of insert
 *	erase                           -> erase one or range of elements, ex : v.erase(v.begin() + 5) <- erase 6th one, v.erase(v.begin(), v.begin() + 3) <- 1st 3
 *	push_back                       -> adds an element to the end
 *	emplace_back                    -> constructs an element in-place at the end
 *	pop_back                        -> removes the last element. ex - in vector size will reduce by 1 but capacity remains same
 *	push_front			-> add an element to the front
 *	emplace_front                   -> construct an element in-place at the front
 *  pop_front                       -> removes the first element
 *  insert_after                    -> inserts elements after an element
 *  emplace_after                   -> constructs elements in-place after an element
 *  erase_after                     -> erases an element after an element		
 *	resize                          -> resize the container. ex - vector, v.resize(n) or v.resize(n, 10)
 *						if n < v.size() , then v.size() = n, remaining elements will be discarded, capacity remains same
 *						if n > v.size() , then v.size() = n, new elements will be added and initialized with given value, else with 0
 *	top				-> access the last element
 *	push				-> stack - inserts element at the top. queue - inserts element at the end
 *	pop				-> stack - removes the top element.    queue - removes the first element
 *	insert_or_assign                -> inserts an element or assigns to the current element if the key already exists
 *	emplace_hint                    -> constructs elements in-place using a hint
 *	try_emplace                     -> inserts in-place if the key does not exist, does nothing if the key exists
 *	extract                         -> extracts nodes from the container
 *	merge                           -> splices nodes from another container
 *
 * Lookup:
 * 	count				-> returns the number of elements matching specific key
 * 	find				-> finds element with specific key
 * 	contains			-> checks if the container contains element with specific key
 * 	equal_range                     -> returns range of elements matching a specific key
 * 	lower_bound			-> returns an iterator to the first element not less than the given key
 * 	upper_bound			-> returns an iterator to the first element greater than the given key
 *
 * Observers:
 * 	key_comp			-> returns the function that compares keys
 * 	value_comp			-> returns the function that compares keys in objects of type value_type
 * 	key_eq                          -> returns the function used to compare keys for equality
 * 	hash_function                   -> returns function used to hash the keys
 *
 * Bucket Interface:
 * 	begin,end,cbegin,cend           -> returns an iterator to the end of the specified bucket
 * 	bucket_count                    -> returns the number of buckets
 * 	max_bucket_count                -> returns the maximum number of buckets
 * 	bucket_size                     -> returns the number of elements in specific bucket
 * 	bucket                          -> returns the bucket for specific key
 *
 * Hash Policy:
 * 	load_factor                     -> returns average number of elements per bucket
 * 	max_load_factor                 -> manages maximum average number of elements per bucket
 * 	rehash                          -> reserves at least the specified number of buckets. This regenerates the hash table
 * 	reserve                         -> reserves space for at least the specified number of elements. This regenerates the hash table
 *
 * Operations:
 *	fill                            -> fill the container with specified value 
 *	swap                            -> swaps the contents of two arrays
 * 	sort, sort(compare_function)    -> sort the elements by library algorithm / own algorithm
 * 	reverse                         -> reverses the order of elements
 * 	merge                           -> merge two sorted lists
 * 	unique                          -> removes consecutive duplicate elements
 * 	remove, remove_if               -> removes elements / removes elements satisfying specific criteria
 * 	splice                          -> transfers the elements from one list to another
 * 	splice_after                    -> moves elements from another forward_list 
 *
 *
 *
 * ****************************************************** Array ************************************************************************
 *
 * description      : fixed size
 * header           : <array>
 * definition       : template< class T, std::size_t N > struct array;
 * syntax           : array<object_type, array_size> array_name
 * type             : sequence container
 * algorithm        : Sequence array
 *
 * member functions : constructor, destructor, operator=
 * element access   : at, operator[], front, back, data
 * capacity         : empty, size, max_size
 * modifiers        : 
 * operations       : fill, swap
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 * ex : array_test()
 *
 ** ****************************************************** Vector ************************************************************************
 *
 * description      : dynamic array
 * header           : <vector>
 * definition       : template<class T, class Allocator = std::allocator<T>> class vector;
 * syntax           : vector<object_type> vector_name
 * type             : sequence container
 * algorithm        : when size == capacity, allocate new memory by doubling the previous capacity. copy elements to new memory and then free the old memory.
 *
 * member functions : constructor, destructor, operator=, assign, get_allocator
 * element access   : at, operator[], front, back, data
 * capacity         : empty, size, max_size, reserve, capacity, shrink_to_fit
 * modifiers        : clear, insert, emplace, erase, push_back, emplace_back, pop_back, resize, swap
 * operations       :  
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 * ex:
 * 	v.insert(5)                                       <- insert a element  in the beginning with value as 5
 * 	v.insert(5, 10)                                   <- insert 5 elements in the beginning with value as 10
 * 	v.insert(v.begin()+2, 5)                          <- insert a element  before iter+2 position with value as 5
 * 	v.insert(v.begin(), v1.begin(), v1.end() )        <- insert a vector v1 in the beginning of vector v, with vector v1 values
 *
 *
 * std::remove and vector::erase idiom
 *
 * 	std::erase()  -> erase the element, which requires a costlier reallign the elements, vector size will change
 * 	std::remove() -> it doesn't delete anything, just reordering the container values and returns the end iterator of ordered one, so that it can be passed to erase for delete at one shot
 *
 * 	vector<int> v = {1,2,5,2,9,5,6,2,5,1,6,7,8,5}
 * 	std::remove(5) || std::remove(v.begin(), v.end(), 5)
 *	v will be       {1 2 2 9 6 2 1 6 7 8 6 7 8 5}
 * 	v.erase( std::remove( v.begin(), v.end(), 5 ), v.end() );
 * 	v will be       {1 2 2 9 6 2 1 6 7 8} 
 * 
 * How to use vector efficiently?
 * 	i. Vector will be more efficient if elements are inserted or removed from the back-end only. Erase/Insert in the begiining/middle require shifting the elements
 * 	ii.Set the storage of vector initially using reserve() member function
 * 	iii.Instead of adding single element in multiple calls, large set of elements is added in single call	
 *
 * ex : vector_test()
 *
 * ********************************************************* List ************************************************************************
 *
 * description      : Doubly linked list
 * header           : <list>
 * definition       : template<class T, class Allocator = std::allocator<T>> class list;
 * syntax           : list<object_type> list_name
 * type             : sequence container
 * algorithm        : Doubly linked list
 *
 * member functions : constructor, destructor, operator=, assign, get_allocator
 * element access   : front, back
 * capacity         : empty, size, max_size
 * modifiers        : clear, insert, emplace, erase, push_back, emplace_back, pop_back, push_front, emplace_front, pop_front, resize, swap
 * operations       : sort, merge, reverse, unique, remove, remove_if, splice
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 * ex : list_test()
 *
 * ********************************************************* Forward List ******************************************************************
 *
 * description      : Singly linked list
 * header           : <forward_list>
 * definition       : template<class T, class Allocator = std::allocator<T>> class forward_list;
 * syntax           : forward_list<object_type> list_name
 * type             : sequence container
 * algorithm        : Singly linked list
 *
 * member functions : constructor, destructor, operator=, assign, get_allocator
 * element access   : front
 * capacity         : empty, max_size
 * modifiers        : clear, insert_after, emplace_after, erase_after, push_front, emplace_front, pop_front, resize, swap
 * operations       : sort, merge, reverse, unique, remove, remove_if, splice_after
 * iterator         : begin, end, cbegin, cend, before_begin, cbefore_begin
 *
 *
 * ********************************************************* Deque ******************************************************************
 *
 * description      : Double ended queue
 * header           : <deque>
 * definition       : template<class T, class Allocator = std::allocator<T>> class deque;
 * syntax           : deque<object_type> deque_name
 * type             : sequence container
 * algorithm        : A map pointed to multiple chunk of arrays. Array size is constant.
 *                    Array blocks are created/deleted in the need basis and array block details stored in a map.
 *
 * member functions : constructor, destructor, operator=, assign, get_allocator
 * element access   : at, operator[], front, back 
 * capacity         : empty, size, max_size, shrink_to_fit
 * modifiers        : clear, insert, emplace, erase, push_back, emplace_back, pop_back, push_front, emplace_front, pop_front, resize, swap
 * operations       : 
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 * ex : deque_test()
 * 
 * ********************************************************* Stack ******************************************************************
 *
 * description      : Stack implementation
 * header           : <stack>
 * definition       : template<class T,class Container = std::deque<T>> class stack;
 * syntax           : stack<object_type> stack_name
 * type             : container adaptor
 * algorithm        : LIFO or FILO ( Last In First Out or Fisrt In Last Out)
 *
 * member functions : constructor, destructor, operator=
 * element access   : top 
 * capacity         : empty, size
 * modifiers        : push, emplace, pop, swap
 * operations       : 
 * iterator         :
 *
 * ex : stack_test()
 * 
 * ********************************************************** Queue ******************************************************************
 *
 * description      : Queue implementation
 * header           : <queue>
 * definition       : template<class T,class Container = std::deque<T>> class queue;
 * syntax           : queue<object_type> queue_name
 * type             : container adaptor
 * algorithm        : FIFO or LILO  ( First In First Out or Last In Last Out )
 *
 * member functions : constructor, destructor, operator=
 * element access   : front, back 
 * capacity         : empty, size
 * modifiers        : push, emplace, pop, swap
 * operations       : 
 * iterator         :  
 *
 * ex : queue_test()
 * 
 * ********************************************************** Priority Queue ******************************************************************
 *
 * description      : A priority queue is a container adaptor that provides constant time lookup of the largest (by default) element
 * header           : <queue>
 * definition       : template< class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>> class priority_queue;
 * syntax           : priority_queue<object_type> queue_name
 * type             : container adaptor
 * algorithm        : Binary heap max
 *
 * member functions : constructor, destructor, operator=
 * element access   : top
 * capacity         : empty, size
 * modifiers        : push, emplace, pop, swap
 * operations       : 
 * iterator         :  
 *
 * ex : priority_queue_test()
 *
 ************************************************************ Map and MultiMap *******************************************************************
 *
 * description      : map is a sorted associative container that contains key-value pairs with unique keys.
 * header           : <map>
 * definition       : template< class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T> >> class map;
 * syntax           : map<key_type, object_type> map_name
 * type             : associative container
 * algorithm        : RedBlock Tree
 *
 * member functions : constructor, destructor, operator=, get_allocator
 * capacity         : empty, size, max_size
 * modifiers        : clear, insert, insert_or_assign, emplace, emplace_hint, try_emplace, erase, swap, extract, merge
 * lookup           : at, operator[], count, find, contains, equal_range, lower_bound, upper_bound
 * observers        : key_comp, value_comp
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 *
 * multimap:
 *
 * 	- same as map, except multimap allows duplicate keys
 * 	- As it allows duplicate keys, it doesn't support at() and operator[] for element access and insert_or_assign() and try_emplace() for modifiers.
 * 	- template< class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T> >> class multimap 
 *
 * ex : map_test()
 * 
 ************************************************************* Set and MultiSet ***************************************************************
 *
 * description      : set is an associative container that contains a sorted set of unique objects of type Key
 * header           : <set>
 * definition       : template< class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>> class set;
 * syntax           : set<object_type> set_name
 * type             : associative container
 * algorithm        : RedBlock Tree
 *
 * member functions : constructor, destructor, operator=, get_allocator
 * capacity         : empty, size, max_size
 * modifiers        : clear, insert, emplace, emplace_hint, erase, swap, extract, merge
 * lookup           : count, find, contains, equal_range, lower_bound, upper_bound
 * observers        : key_comp, value_comp
 * iterator         : begin, end, cbegin, cend, rbegin, rend, crbegin, crend
 *
 *
 * multiset:
 * 	- same as set, except it allows duplicate keys
 * 	- template< class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>> class multiset
 *
 *
 * ex : set_test()
 *
 * ******************************************************** Unordered Map and MultiMap **********************************************************
 *
 * description      : Unordered map is an associative container that contains key-value pairs with unique keys
 * header           : <map>
 * definition       : template< class Key, class T, class Hash=std::hash<Key>, class KeyEqual=std::equal_to<Key>,
 * 			class Allocator = std::allocator< std::pair<const Key, T> >> class unordered_map;
 * syntax           : unordered_map<key_type, object_type> unordered_map_name
 * type             : unordered associative container
 * algorithm        : Hash table
 *
 * member functions : constructor, destructor, operator=, get_allocator
 * capacity         : empty, size, max_size
 * modifiers        : clear, insert, insert_or_assign, emplace, emplace_hint, try_emplace, erase, swap, extract, merge
 * lookup           : at, operator[], count, find, contains, equal_range
 * bucket interface : begin, cbegin, end, cend, bucket_count, max_bucket_count, bucket_size, bucket
 * hash policy      : load_factor, max_load_factor, rehash, reserve
 * observers        : hash_function, key_eq
 * iterator         : begin, end, cbegin, cend
 *
 *
 * unordered_multimap:
 *
 * 	- same as unordered map, except unordered multimap allows duplicate keys
 * 	- As it allows duplicate keys, it doesn't support at() and operator[] for element access and insert_or_assign() and try_emplace() for modifiers.
 * 	- unordered_multimap<key_type, object_type> unordered_multimap_name
 * 
 ************************************************************ Unordered Set and Multiset **********************************************************
 *
 * description      : Unordered set is an associative container that contains a set of unique objects of type Key. Search
 * header           : <set>
 * definition       : template<class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<Key>> class unordered_set;
 * syntax           : unordered_set<object_type> unordered_set_name
 * type             : unordered associative container
 * algorithm        : Hash table
 *
 * member functions : constructor, destructor, operator=, get_allocator
 * capacity         : empty, size, max_size
 * modifiers        : clear, insert, emplace, emplace_hint, erase, swap, extract, merge
 * lookup           : count, find, contains, equal_range
 * bucket interface : begin, cbegin, end, cend, bucket_count, max_bucket_count, bucket_size, bucket
 * hash policy      : load_factor, max_load_factor, rehash, reserve
 * observers        : hash_function, key_eq
 * iterator         : begin, end, cbegin, cend
 *
 *
 * unordered_multiset:
 *
 * 	- same as unordered set, except unordered multiset allows duplicate keys
 * 	- unordered_multiset<object_type> unordered_multiset_name
 *
 * ******************************************************************************************************************************
 *
 * Which one to use ?
 *
 * array vs vector
 * 	array  - fixed size array
 * 	vector - dynamic size array, will double it's size when ever size = capacity
 *
 * vector vs list
 * 	vector - provides random access operator[]
 * 	       - much efficient if insert or delete are only in the end
 * 	       - insert or delete any place other than end, requires costlier reallignment
 * 	       - memory reallocation and copy the complete vector opeartion required size = capacity
 * 	list   - doesn't provide's random access
 * 	       - insert/delete is easy, no reallignment
 *
 * vector vs map
 * queue vs deque
 * 	queue - we can insert only in one end and remove from other end
 * 	deque - insert and remove from both the ends .
 * 	      - Provides random access operator[] like vector
 * 	      - So using deque , we can model both stack and queue
 * map vs set
 * map vs multimap
 * map vs unordered_map
 *
 *
 *
 *
 *
 *
 *
 *
 * *************************************************************************************************************************************/

#include <iostream>
#include <string>
#include <utility>
#include <tuple>
#include <array>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <list>

using namespace std;

void pair_tuble_test();
void array_test();
void print_array(auto &arr);
void iterator_test();
void vector_test();
void map_test();
void list_test();


int main(){

	//pair_tuble_test();
	//array_test();
	//iterator_test();
	//vector_test();
	//map_test();
	list_test();

	return 0;
}


void pair_tuble_test() {

	std::pair<std::string, double> product1;                                  // default constructor
	product1 = std::make_pair("OnePlus6", 35000.00);

	std::cout << product1.first << " : " << product1.second << std::endl;
	product1.second = product1.second - 1000;
	std::cout << product1.first << " : " << product1.second << std::endl;

	std::pair<std::string, double> product2("iPhone", 50000.00);              // parameterized constructor
	std::pair<std::string, double> product3(product2);                        // copy constructor

	std::cout << product2.first << " : " << product2.second << std::endl;
	std::cout << product3.first << " : " << product3.second << std::endl;

	// pair assignment
	product3 = product1;
	std::cout << product3.first << " : " << product3.second << std::endl;

	// pair relational operators  == , != , > , < , <= , >=
	if ( product1 == product3)
		std::cout << "product and product3 is equal" << std::endl;

	// pair swapping - use to swap values of two pairs
	std::cout << "Before swap" << std::endl;
	std::cout << "product 2 -> " << product2.first << " : " << product2.second << std::endl;
	std::cout << "product 3 -> " << product3.first << " : " << product3.second << std::endl;
	product2.swap(product3);
	std::cout << "After swap" << std::endl;
	std::cout << "product 2 -> " <<  product2.first << " : " << product2.second << std::endl;
	std::cout << "product 3 -> " << product3.first << " : " << product3.second << std::endl;

	// tuple test 
	std::tuple<std::string, int, double> product4 = std::make_tuple("onePlus", 6, 35999.99);
	std::tuple<int, int, double> product5(1, 6, 35999.99);
	std::cout << "product 4 -> " << std::get<0>(product4) << " , version : " << std::get<1>(product4) << " , price : " << std::get<2>(product4) << std::endl;
	std::get<2>(product4) = std::get<2>(product4) - 1000;
	std::cout << "product 4 -> " << std::get<0>(product4) << " , version : " << std::get<1>(product4) << " , price : " << std::get<2>(product4) << std::endl;

	std::string name;
	int version, price;
	std::tie(name, version, price) = product4;
	std::cout << "product 4 -> " << name << " , version : " << version << " , price : " << price << std::endl;
	std::cout << std::get<0>(product1) << std::endl;
	std::tie(name, price) = product1;

	std::tuple_element<0,decltype(product1)>::type productName = std::get<0>(product1);
	std::cout << productName << std::endl;
	std::cout << std::tuple_size<decltype(product4)>::value << std::endl;

}


void array_test() {

	std::array<int, 6> array1 = {1,2,3,4};
	std::array<int, 6> array2 = {5,6,7,8};
	std::array<int, 7> array3;

	std::cout << "arr.at(0)             : " << array1.at(0) << std::endl;
	std::cout << "arr[0]                : " << array1[0] << std::endl;
	std::cout << "arr.front()           : " << array1.front() << std::endl;
	std::cout << "arr.back()            : " << array1.back() << std::endl;
	std::cout << "arr.data()            : " << array1.data() << std::endl;

	std::cout << "arr.size()            : " << array1.size() << std::endl;
	std::cout << "arr.max_size()        : " << array1.max_size() << std::endl;
	std::cout << "arr.empty()           : " << array1.empty() << std::endl;

	array1.fill(1);
	std::cout << "arr.at(0)             : " << array1.at(0) << std::endl;
	array1.swap(array2);
	std::cout << "arr.at(0)             : " << array1.at(0) << std::endl;
	std::swap(array1, array2);
	std::cout << "arr.at(0)             : " << array1.at(0) << std::endl;
	print_array(array2);

	//std::array<int, 4>::iterator i;
	for ( auto i = array2.begin(); i != array2.end(); i++)
		std::cout << *i << " ";
	std::cout << std::endl;

	for ( auto i = array2.rbegin(); i != array2.rend(); i++)
		std::cout << *i << " ";
	std::cout << std::endl;


}

void print_array(auto &arr){
	for ( auto &e : arr )
		std::cout << e << " ";
	std::cout << std::endl;
}

/*template<class Container>
void print(const Container &container){
	Container::iterator i = container.begin();
	while( i != container.end() )
		std::cout << *i++ << " ";
	std::cout << std::endl;
}*/

void iterator_test() {

	std::vector<int> v;
	std::vector<int>::iterator i;

	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	v.push_back(40);
	v.push_back(50);
	v.push_back(60);
	v.push_back(70);
	v.push_back(80);

	for ( i = v.begin(); i != v.end(); i++)
		std::cout << *i << " ";
	std::cout << std::endl;

	std::cout << "distance(v.begin(), v.end()) : " << std::distance(v.begin(), v.end()) << std::endl;

	i = v.begin();
	std::advance(i, 4);
	for ( auto it = i; it != v.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;

	i = v.end();
	std::advance(i, -6);
	for ( auto it = i; it != v.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	
	i = v.begin();
	for ( auto it = std::next(i, 3); it != v.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;

}

template<typename T> void print_queue(T& q) {
    while(!q.empty()) {
        std::cout << q.top() << " ";
        q.pop();
    }
    std::cout << '\n';
}



void vector_test() {

	std::vector<int> v;
	std::vector<int>::iterator i;

	std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;

	{
		std::vector<int> v;
		for ( int i = 0; i <= 10; i++) {
			std::cout << "i = " << i << " , vect size , capacity : " << v.size() << " , "  << v.capacity() << std::endl;
			v.push_back(i);
		}
	}

	v.reserve(15);
	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	v.push_back(40);
	v.push_back(50);
	v.push_back(60);
	v.push_back(70);
	v.push_back(80);
	v.push_back(90);
	v.push_back(100);
	
	std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;
	v.shrink_to_fit();
	std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;
	v.push_back(90);
	std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;
	v.pop_back();

	for ( i = v.begin(); i != v.end(); i++)
		std::cout << *i << " ";
	std::cout << std::endl;

	int size = v.size();
	while ( size-- > 0 ) {
		std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;
		v.pop_back();
	}
	//v.clear();
	std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;

	size = 1;
	while ( size <= 10 ) {
		std::cout << "vect size , capacity , max_size : " << v.size() << " , "  << v.capacity() << " , "  << v.max_size() << std::endl;
		v.push_back(size++);
	}
	print_array(v);
	std::cout << "vect size , capacity : " << v.size() << " , "  << v.capacity() << std::endl;
	v.resize(4);
	print_array(v);
	std::cout << "vect size , capacity : " << v.size() << " , "  << v.capacity() << std::endl;
	v.resize(101);
	print_array(v);
	std::cout << "vect size , capacity : " << v.size() << " , "  << v.capacity() << std::endl;

	std::vector<int> v1 = {1,2,5,2,9,5,6,2,5,1,6,7,8,5};
	print_array(v1);
	v1.erase( std::remove(v1.begin(), v1.end(), 5), v1.end() );
	print_array(v1);

	std::priority_queue<int> pq;
	pq.push(10);
	pq.push(30);
	pq.push(20);
	print_queue(pq);
	print_queue(pq);


}


class Employee {
private:
	int _id;
	string _name;
public:
	void print() { cout << "id : " << _id << " , name : " << _name << endl; }
	Employee(int id = 0, string name = "name") : _id(id), _name(name) { cout << "constructor called" << endl ; }
	~Employee() { cout << "destructor called" << endl; }
};

class Compare {
	public :
		bool operator()(int l, int r) const {
			return l > r;
		}
};

void map_test() {


	//map<int, Employee> employees;
	map<int, Employee, Compare> employees;

	Employee emp1(1, "kanna");
	Employee emp2(2, "ganesh");
	Employee emp3(3, "karthi");

	employees.insert(pair<int, Employee>(1, emp1));
	employees.insert(pair<int, Employee>(2, emp2));
	employees.emplace(3, emp3);

	for ( auto &emp : employees)
		emp.second.print();

	employees.at(1).print();
	employees[2].print();

	cout << "map size : " << employees.size() << " , max_size() : " << employees.max_size() << endl;

	employees.erase(3); // erasing by key, can be erase by iterator and iterator range as well

	for ( auto it = employees.begin(); it != employees.end(); it++)
		it->second.print();

	auto it = employees.find(2);
	if ( it != employees.end() ) {  // data found
		it->second.print();
	}

	cout << "total record with key 2 : " << employees.count(2) << endl;

}

void print_list(list<int> l) {

	for ( auto it = l.begin(); it != l.end(); it++) {
		cout << *it << endl;
	}

}

void list_test() {

	list<int> l;
	Compare compare;

	for ( int i = 1; i < 10; i++)
		l.insert(l.begin(), i);

	print_list(l);

	l.sort();
	print_list(l);
	l.sort(compare);
	print_list(l);
}
