/********************************** PImpl Idiom in C++ **************************
 *
 * g++ -std=c++17 API.cpp main.cpp -o api
 *
 * PImpl - Pointer to IMPLementation
 *
 * The main point of the pimpl idiom is to hide the implementation, especially member variables and private methods.
 *
 * How to implement :
 *
 * 1. In the header, forward declare a nested class Impl, and hold a std::unique_ptr<Impl> as the only data member
 * 2. In the cpp file, define Impl with all the private members
 * 3. Public methods forward their work to impl_
 * 4. Define the special members in the cpp file, where Impl is a complete type
 *      Destructor is a must : the unique_ptr deleter needs the full Impl, so '= default' in the header doesn't compile
 *      Move                 : declaring a destructor stops the compiler generating it, so ask for it with '= default'
 *      Copy                 : unique_ptr can't be copied, so write a deep copy if the class needs one
 *
 * Advantages of PImpl:
 *
 * Binary Compatibility: The class is always one pointer in size, so changing private members doesn't break the dependent code.
 *                       Changing the public methods still does.
 *
 * Compilation time: Only the implementation file needs to be rebuilt when private members change, not every client.
 *
 * Data Hiding: Hides internal details, and keeps the headers of the libraries used by the implementation out of the public header.
 *
 * Disadvantages of PImpl:
 *
 * Runtime cost: One heap allocation per object, and one more pointer jump on each call, which can miss the cache.
 *               Avoid it on low latency hot paths.
 *
 * Moved from object: impl_ is null, so it can only be destroyed or assigned a new value.
 *
 * Const: A const method can still modify *impl_, as const applies to the pointer and not to what it points to.
 *
 ******************************************************************************/

#ifndef API_H
#define API_H

#include <memory>

class API {
	public:
		API(int a, int b);
		~API();

		API(const API& other);
		API& operator=(const API& other);
		API(API&& other) noexcept;
		API& operator=(API&& other) noexcept;

		void set_a(int a);
		void execute() const;
	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
};

#endif
