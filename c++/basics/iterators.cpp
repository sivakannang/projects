/****************************** Iterators *************************************
* 
* - Iterators are pointer-like objects to traverse containers.
* - STL containers provide begin(), end(), rbegin(), rend(), cbegin(), cend().
* - Five categories: Input, Output, Forward, Bidirectional, Random Access.
* - Use auto keyword to simplify iterator declarations.
* - Prefer range-based for loops for readability.

* Iterator Types:
* ---------------
* Input Iterator         - Read-only, single-pass (e.g., istream_iterator)
* Output Iterator        - Write-only, single-pass (e.g., ostream_iterator)
* Forward Iterator       - Read/write, one direction (e.g., forward_list)
* Bidirectional Iterator - ++ and -- supported (e.g., list, set, map)
* Random Access Iterator - Full pointer support: +, -, [] (e.g., vector, array)

*************************   Interview Questions and Answers *******************************
*
* 1. What are the different types of iterators in C++? Explain their use cases.
*    - Input, Output, Forward, Bidirectional, Random Access. Each type provides increasing functionality. Input/output are for single-pass; random access supports full pointer arithmetic.
*
* 2. What is the difference between const_iterator and iterator?
*    - const_iterator does not allow modification of the elements it points to, while iterator does.
*
* 3. How does a reverse_iterator work internally?
*    - It wraps a regular iterator and iterates backwards using `--base()` on increment and `++base()` on decrement.
*
* 4. Which containers provide random access iterators?
*    - vector, array, deque.
*
* 5. Can you implement a custom iterator for a user-defined class?
*    - Yes, by creating a class that overloads `operator*`, `operator++`, and comparison operators, you can define custom iterators.
*
* 6. What is the difference between iterator and pointer?
*    - Iterators are abstract and work with containers, can be safer and more flexible than raw pointers which directly reference memory.
*
* 7. What are the benefits of using iterators over indexing?
*    - Iterators allow generic programming, can work across all container types, and are often more readable and flexible.
*
* 8. How does an iterator become invalidated?
*    - When the container is modified (like insertions or deletions), existing iterators may become invalid and unsafe to use.
*
* 9. What happens if we modify a container while iterating through it?
*    - It may lead to undefined behavior or iterator invalidation; it is safer to collect changes and apply after iteration.
*
* 10. How do standard algorithms use iterators?
*     - Algorithms like sort, find, copy operate on iterator ranges [begin, end) to remain container-agnostic.
*
*********************************************************************************************************************************/

#include <iostream>
#include <vector>
#include <algorithm>

int main() {
	std::vector<int> v = {10, 20, 30, 40, 50};

	// 1. Normal Iterator
	std::cout << "Normal Iterator: ";
	for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;

	// 2. Const Iterator
	std::cout << "Const Iterator: ";
	for (std::vector<int>::const_iterator it = v.cbegin(); it != v.cend(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;

	// 3. Reverse Iterator
	std::cout << "Reverse Iterator: ";
	for (auto it = v.rbegin(); it != v.rend(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;

	// 4. Range-based For Loop (modern C++)
	std::cout << "Range-based For: ";
	for (int val : v) {
		std::cout << val << " ";
	}
	std::cout << std::endl;

	// 5. Using Iterator with Algorithms
	std::sort(v.begin(), v.end());
	std::reverse(v.begin(), v.end());
	std::cout << "After sort and reverse: ";
	for (auto it = v.begin(); it != v.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;

	return 0;
}

