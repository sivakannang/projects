
/*---------------------------------------------------------

✅ C++ Memory Layout:

When you run a C++ program, the operating system divides its memory into regions:

 High Memory Addresses
+---------------------+
| Stack               |  <- local_var, automatically managed(allocated/deallocated), fast but limit in size
+---------------------+
| (Unused gap)        |
+---------------------+
| Heap                |  <- dynamic memory allocation using new/delete, must be managed by programmer. heap_var - pointer on stack, points to heap
+---------------------+
| BSS Segment         |  <- uninitialized globals/statics
+---------------------+
| Data Segment        |  <- initialized globals/statics
+---------------------+
| Read-Only Data      |  <- string literals, const globals
+---------------------+
| Text Segment        |  <- machine code
+_____________________+
 Low Memory Addresses


- Stack overflow:
    - infinite recursion
- Heap overflow:
    - writing past allocated memory
    - can cause corruption

---------------------------------------------------------
✅ sizeof and Alignment - Data alignment affects structure size.

#pragma pack(push, 1) // save current packing and set to 1
    struct MyStruct {
        char a;  // 1 byte
		 // 3 bytes padding, if we don't use pragma pack(push, 1)
        int b;   // 4 byte
    };
#pragma pack(pop) // restore previous packing

---------------------------------------------------------
   Dynamic Memory Allocation in C++
---------------------------------------------------------

   - new/delete , new[]/delete[] (preferred in C++)
   - Use smart pointer for automatically manage heap memory and avoid leaks, instead of new/delete
   - new/delete call constructors/destructors; malloc/free do not
   - Calling delete on nullptr does nothing, but calling delete twice on the same valid pointer causes undefined behavior & may crash or corrupt memory

   How can we restrict a class to be created only on the stack?
      - delete its operator new/operator new[] and placement new versions.
   
   How can we restrict a class to be created only on the heap?
      - Move constructor and destructor to private and expose wrapper function for create/destroy object
      - Also delete copy/move constructors/assignments for avoid copy

   How to read const pointers ?
      - const binds to the thing on its left; if nothing on the left, it binds to the thing on its right.
      - const int *ptr; [ nothing on left so const binds with right int, means pointer is mutable, value is immutable ]
      - int const *ptr; [ const binds with left side int, means pointer is mutable, value is immutable ] 
      - int *const ptr; [ const binds with left side *, means pointer is immutable, value is mutable ]
      - int const * const ptr; [ const binds with left side int, then 2nd const binds with left side *, so both pointer and value are immuatble ] 
      - const int* const ptr;  [ nothing on left, so const binds with right side int, 2nd const binds with left side *, so both are immutable ]
---------------------------------------------------------
   Smart Pointers in C++  <memory>
---------------------------------------------------------

  - Smart pointers automate memory management by releasing memory when no longer needed.
  	- std::unique_ptr: sole ownership, cannot be copied.
	- std::shared_ptr: reference-counted shared ownership.
	- std::weak_ptr: non-owning reference to a shared_ptr.

  - Prefer smart pointers over raw new/delete for exception safe memory management

  - Avoid new operator and use use std::make_unique() and std::make_shared() for exception safe construction


Questions:

// 1. What is the purpose of smart pointers?
//    - To manage dynamic memory automatically and safely.

// 2. When should std::unique_ptr be used?
//    - When exclusive ownership is needed.

// 3. What does std::shared_ptr do?
//    - Shares ownership; memory is freed when the last shared_ptr is destroyed.

// 4. Why use std::weak_ptr?
//    - To avoid cyclic references when using shared_ptr.

// 5. What functions are used to create smart pointers?
//    - std::make_unique and std::make_shared.


---------------------------------------------------------
   Important Smart Pointer Functions
---------------------------------------------------------

// std::unique_ptr
// - get()         -> returns the raw pointer
// - release()     -> releases ownership and returns raw pointer, this raw ptr should be deleted manually or should pass this raw ptr to another unique_ptr
// - reset(ptr)    -> deletes current and takes ownership of new pointer
// - swap()        -> swaps with another unique_ptr

// std::shared_ptr
// - get()         -> returns the raw pointer
// - use_count()   -> returns reference count
// - unique()      -> true if use_count() == 1
// - reset(ptr)    -> replaces managed object
// - swap()        -> swaps with another shared_ptr

// std::weak_ptr
// - lock()        -> returns shared_ptr if object is still alive
// - expired()     -> checks whether object is deleted
// - reset()       -> resets the weak_ptr

// template< class T, class Deleter = std::default_delete<T> > class unique_ptr;

// get_deleter()   -> returns the deleter that is used for destruction of the managed object
                   -> get_deleter returns a pointer to the deleter in shared_ptr, but reference to the deleter in unique_ptr
		   -> Deleter must be FunctionObject or lvalue reference to a FunctionObject


// std::unique_ptr<int>      p1(new int);
// std::unique_ptr<int[]>    p2(new int[50]);
// std::unique_ptr<Object>   p3(new Object("Lamp"));

// std::unique_ptr<int>      p1 = std::make_unique<int>();
// std::unique_ptr<int[]>    p2 = std::make_unique<int[]>(50);
// std::unique_ptr<Object>   p3 = std::make_unique<Object>("Lamp");
// std::unique_ptr<Object[]> p4 = std::make_unique<Object[]>(50);


// we can't construct array directly in make_shared(), we have to use new[], refer below shared_ptr array example

// std::shared_ptr<int>      p1 = std::make_shared<int>();
// std::shared_ptr<Object>   p2 = std::make_shared<Object>("Lamp");
// std::shared_ptr<Student>   student(new Student[3], [](int *p) { delete [] p; });    // Until C++14
// std::shared_ptr<Student[]> student(new Student[3])                                  // From C++17
// auto v = std::make_shared<std::vector<int>>(10); // 10 elements, value-initialized
 
// std::shared_ptr<int>  p3(new int[50], [](int *p) { delete[] p;} );   // Until C++17, for shared pointer arrays we can't use int[] ,we should specify int only
// std::shared_ptr<Student> student(new Student[3], [](Student *s) {delete[] s;});
        - Until C++17, shared pointer array calls delete instead of delete[], so need to write custom deletor . Also can't use make_shared() here, as it doesn't support customized deleter here


//  void function(std::unique_ptr<A>(new A()), std::unique_ptr<B>(new B())) { ... }
//  Suppose that new A() succeeds, but new B() throws an exception , memory silently leakes for A() , no way to clean it up.
//  But by wrapping A and B into std::make_uniques you are sure the leak will not occur
//  Prefer using make_unique() or make_shared() , avoid using new()

// for access array version ->    student.get()[idx] , for member access student.get()[idx].id


---------------------------------------------------------
   References in C++
---------------------------------------------------------

// A reference is an alias for an existing variable.
// Syntax: type& name = original;

// - Must be initialized when declared.
// - Cannot be null (unlike pointers).
// - Used for function parameter passing and return types.

// Types:
// - Lvalue references (T&)
// - Const lvalue references (const T&)
// - Rvalue references (T&&) - introduced in C++11 for move semantics.

Questions:

// 1. What is a reference?
//    - An alias for another variable.

// 2. Can references be reseated?
//    - No, once initialized, they cannot be changed to refer to another object.

// 3. Difference between pointer and reference?
//    - Pointers can be reassigned and null; references cannot.

// 4. What is the use of rvalue references?
//    - Enables move semantics and perfect forwarding.

// 5. Can a function return a reference?
//    - Yes, it can return a reference to a variable with suitable lifetime.

*******************************************************************/

#include <iostream>
#include <memory>
#include <cassert>

// Sample: Class restricted to stack only
class StackOnly {
	public:
		void show() { std::cout << "Created on stack only\n"; }

	private:
		// Prevent scalar and array heap allocation
		void* operator new(size_t) = delete;
		void operator delete(void*) = delete;
		void* operator new[](size_t) = delete;
		void operator delete[](void*) = delete;
};

// Sample: Class restricted to heap only
class HeapOnly {
	public:
		static HeapOnly* create() { return new HeapOnly(); }
		static void destroy(HeapOnly* ptr) { delete ptr; }

		void show() { std::cout << "Created on heap only\n"; }

	private:
		HeapOnly() = default;
		~HeapOnly() = default;

		// Prevent copy
		HeapOnly(const HeapOnly&) = delete;
		HeapOnly& operator=(const HeapOnly&) = delete;
};


template <typename T>
T** allocate_2d(int rows, int cols) {
	T** arr = new T*[rows];
	for (int i = 0; i < rows; ++i)
		arr[i] = new T[cols];
	return arr;
}

template <typename T>
void free_2d(T** arr, int rows) {
	for (int i = 0; i < rows; ++i)
		delete[] arr[i];
	delete[] arr;
}

template <typename T>
T*** allocate_3d(int depth, int rows, int cols) {
	T*** arr = new T**[depth];
	for (int d = 0; d < depth; ++d)
		arr[d] = allocate_2d<T>(rows, cols);
	return arr;
}

template <typename T>
void free_3d(T*** arr, int depth, int rows) {
	for (int d = 0; d < depth; ++d)
		free_2d(arr[d], rows);
	delete[] arr;
}

template <typename T>
void print_2d(T** arr, int rows, int cols) {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			std::cout << arr[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

template <typename T>
void print_3d(T*** arr, int depth, int rows, int cols) {
	for (int d = 0; d < depth; ++d) {
		std::cout << "Depth " << d << ":";
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				std::cout << arr[d][i][j] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}

void memory_demo() {
	int stack_var = 10; // Stored on stack
	int* heap_var = new int(20); // Allocated on heap
	static int static_var = 30; // Stored in data segment

	std::cout << "Stack: " << stack_var << "\n";
	std::cout << "Heap: " << *heap_var << "\n";
	std::cout << "Data: " << static_var << "\n";

	delete heap_var; // Free heap memory
}

void smart_pointer_demo() {
	// unique_ptr examples
	std::unique_ptr<int> up = std::make_unique<int>(10);
	int* raw_up = up.get();
	std::unique_ptr<int> released_up(up.release());
	released_up.reset(new int(20));
	std::unique_ptr<int> up2 = std::make_unique<int>(30);
	released_up.swap(up2);

	// shared_ptr examples
	std::shared_ptr<int> sp1 = std::make_shared<int>(100);
	std::shared_ptr<int> sp2 = sp1;
	std::cout << "Use count: " << sp1.use_count() << std::endl;
	std::cout << "Is unique? " << std::boolalpha << sp1.unique() << std::endl;
	sp2.reset(new int(200));

	// weak_ptr examples
	std::weak_ptr<int> wp = sp1;
	if (!wp.expired()) {
		std::shared_ptr<int> sp_locked = wp.lock();
		std::cout << "Locked weak_ptr: " << *sp_locked << std::endl;
	}
	wp.reset();

}

// Note - this class doesn't handle array, for handle array need to add template specialization
template <typename T, typename D = std::default_delete<T>>
class uniqueptr {
	T* ptr = nullptr;
	D  del;

	public:
	explicit uniqueptr(T* p = nullptr) noexcept : ptr(p) {}
	explicit uniqueptr(T* p, D d) noexcept : ptr(p), del(std::move(d)) {}
	~uniqueptr() noexcept { if (ptr) del(ptr); }

	uniqueptr(const uniqueptr&) = delete;
	uniqueptr& operator=(const uniqueptr&) = delete;

	uniqueptr(uniqueptr&& o) noexcept : ptr(o.ptr), del(std::move(o.del)) { o.ptr = nullptr; }
	uniqueptr& operator=(uniqueptr&& o) noexcept {
		if (this != &o) { reset(); del = std::move(o.del); ptr = o.ptr; o.ptr = nullptr; }
		return *this;
	}

	T* get() const noexcept { return ptr; }
	explicit operator bool() const noexcept { return ptr != nullptr; }
	T& operator*() const { assert(ptr); return *ptr; }
	T* operator->() const { assert(ptr); return ptr; }

	T* release() noexcept { T* p = ptr; ptr = nullptr; return p; }
	void reset(T* p = nullptr) noexcept { if (ptr != p) { if (ptr) del(ptr); ptr = p; } }
	void swap(uniqueptr& o) noexcept { using std::swap; swap(ptr,o.ptr); swap(del,o.del); }
};

template <typename T, typename... Args>
uniqueptr<T> makeunique(Args&&... args) {
	return uniqueptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
class MySharedPtr {
	T* ptr;
	int* ref_count;

	void release() {
		if (ref_count && --(*ref_count) == 0) {
			delete ptr;
			delete ref_count;
		}
	}

	public:
	explicit MySharedPtr(T* p = nullptr) : ptr(p), ref_count(new int(1)) {}
	~MySharedPtr() { release(); }

	MySharedPtr(const MySharedPtr& other) : ptr(other.ptr), ref_count(other.ref_count) {
		++(*ref_count);
	}

	MySharedPtr& operator=(const MySharedPtr& other) {
		if (this != &other) {
			release();
			ptr = other.ptr;
			ref_count = other.ref_count;
			++(*ref_count);
		}
		return *this;
	}

	T* get() const { return ptr; }
	T& operator*() const { return *ptr; }
	T* operator->() const { return ptr; }
	int use_count() const { return ref_count ? *ref_count : 0; }
};

int main() {
	smart_pointer_demo();
	memory_demo();

	StackOnly s;
	s.show();

	HeapOnly* h = HeapOnly::create();
	h->show();
	HeapOnly::destroy(h);

	return 0;
}
