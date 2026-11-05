// g++ -std=c++17 memory.cpp -o memory

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

- Avoid new operator and use std::make_unique() and std::make_shared()
  * The code never has to pair a new with a delete, and doesn't repeat the type
  * make_shared also puts the object and its reference counts in one allocation. The catch is that
    the memory isn't freed until the last weak_ptr is gone too.


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
// - unique()      -> removed in C++20, so use use_count() == 1. gcc still accepts it, but other compilers don't.
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


// make_shared() can construct arrays only from C++20. Before that, use new[] as in the examples below.

// std::shared_ptr<int>      p1 = std::make_shared<int>();
// std::shared_ptr<Object>   p2 = std::make_shared<Object>("Lamp");
// std::shared_ptr<Student[]> student(new Student[3]);                                 // From C++17
// std::shared_ptr<Student[]> student = std::make_shared<Student[]>(3);                // From C++20
// auto v = std::make_shared<std::vector<int>>(10); // 10 elements, value-initialized

// std::shared_ptr<int>  p3(new int[50], [](int *p) { delete[] p;} );   // Until C++17, shared_ptr had no array form, so specify int rather than int[]
// std::shared_ptr<Student> student(new Student[3], [](Student *s) {delete[] s;});
- Until C++17, a shared_ptr to an array calls delete instead of delete[], so it needs a custom deleter. make_shared() can't take a custom deleter.


//  function(std::unique_ptr<A>(new A()), std::unique_ptr<B>(new B()));
//  Until C++14, this could run new A(), then new B(), and only then the unique_ptr constructors.
//  If new B() threw, A leaked. make_unique closed that gap.
//  C++17 closed it for this code too, as each argument is now fully evaluated before the next one starts.
//  make_unique() and make_shared() are still preferred, for the reasons above.

// for access array version ->    student[idx] , for member access student[idx].id    (operator[] needs the T[] form, from C++17 for shared_ptr)


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

#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

class StackOnly {
public:
    StackOnly() = default;
    void show() { std::cout << "Created on stack only\n"; }

    // 1. Delete standard scalar and array new/delete
    void* operator new(std::size_t) = delete;
    void operator delete(void*) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete[](void*) = delete;

    // 2. Delete placement new (prevents construction in pre-allocated buffers)
    void* operator new(std::size_t, void*) = delete;
    void* operator new[](std::size_t, void*) = delete;
};


class HeapOnly {
public:
    // Factory method for creation
    static HeapOnly* create() {
        return new HeapOnly();
    }

    void destroy() {
        delete this;
    }

    void show() const {
        std::cout << "Object at " << this << " is strictly on the heap.\n";
    }

private:
    // 1. Private Constructor: Prevents direct 'HeapOnly h' or 'new HeapOnly()' externally
    HeapOnly() {
        std::cout << "Constructor called\n";
    }

    // 2. Private Destructor: This is the SECRET SAUCE.
    // Objects on the stack MUST have a public destructor.
    ~HeapOnly() {
        std::cout << "Destructor called\n";
    }

    // 3. Disable Copy and Move: Prevent duplication which could lead to stack copies
    HeapOnly(const HeapOnly&) = delete;
    HeapOnly& operator=(const HeapOnly&) = delete;
    HeapOnly(HeapOnly&&) = delete;
    HeapOnly& operator=(HeapOnly&&) = delete;
};



class HeapOnlySmart {
public:
    // make_unique can't call a private constructor, so this uses new
    static std::unique_ptr<HeapOnlySmart> create() {
        return std::unique_ptr<HeapOnlySmart>(new HeapOnlySmart());
    }

    void show() { std::cout << "Created on heap only, managed by smart pointer\n"; }

private:
    // Private constructor/destructor forces heap allocation through 'new'
    HeapOnlySmart() = default;
    ~HeapOnlySmart() = default;

    // Prevent copy/move, which could create an object outside the heap
    HeapOnlySmart(const HeapOnlySmart&) = delete;
    HeapOnlySmart& operator=(const HeapOnlySmart&) = delete;
    HeapOnlySmart(HeapOnlySmart&&) = delete;
    HeapOnlySmart& operator=(HeapOnlySmart&&) = delete;

    // unique_ptr doesn't call the destructor itself: its deleter does, so that needs the access
    friend struct std::default_delete<HeapOnlySmart>;
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
	std::cout << "unique_ptr get(): " << *up.get() << std::endl;
	std::unique_ptr<int> released_up(up.release());
	released_up.reset(new int(20));
	std::unique_ptr<int> up2 = std::make_unique<int>(30);
	released_up.swap(up2);

	// shared_ptr examples
	std::shared_ptr<int> sp1 = std::make_shared<int>(100);
	std::shared_ptr<int> sp2 = sp1;
	std::cout << "Use count: " << sp1.use_count() << std::endl;
	std::cout << "Is unique? " << std::boolalpha << (sp1.use_count() == 1) << std::endl;
	sp2.reset(new int(200));

	// weak_ptr examples
	std::weak_ptr<int> wp = sp1;
	if (!wp.expired()) {
		std::shared_ptr<int> sp_locked = wp.lock();
		std::cout << "Locked weak_ptr: " << *sp_locked << std::endl;
	}
	wp.reset();

}

namespace siva {

	template <typename T>
		class unique_ptr {
			private:
				T* ptr_;

			public:
				// 1. Constructor
				explicit unique_ptr(T* ptr = nullptr) : ptr_(ptr) {}

				// 2. Destructor
				~unique_ptr() {
					delete ptr_;
				}

				// 3. DISABLE Copying (Crucial for unique_ptr)
				unique_ptr(const unique_ptr&) = delete;
				unique_ptr& operator=(const unique_ptr&) = delete;

				// 4. Move Constructor (Transfer ownership)
				unique_ptr(unique_ptr&& other) noexcept : ptr_(other.ptr_) {
					other.ptr_ = nullptr;
				}

				// 5. Move Assignment
				// Take the new pointer before deleting the old object. In a list,
				// head = std::move(head->next) has other inside the object being deleted.
				unique_ptr& operator=(unique_ptr&& other) noexcept {
					reset(other.release());
					return *this;
				}

				// 6. Nullptr Assignment
				unique_ptr& operator=(std::nullptr_t) {
					reset();
					return *this;
				}

				// 7. Modifiers
				T* release() noexcept {
					T* temp = ptr_;
					ptr_ = nullptr;
					return temp;
				}

				void reset(T* ptr = nullptr) noexcept {
					T* old_ptr = ptr_;
					ptr_ = ptr;
					delete old_ptr;
				}

				// 8. Accessors (Const Correct)
				T& operator*() const { return *ptr_; }
				T* operator->() const { return ptr_; }
				T* get() const { return ptr_; }

				explicit operator bool() const { return ptr_ != nullptr; }
		};



	template<typename T>
		class shared_ptr {
			private:
				T *ptr_;
				int *ref_; // Raw int for single-threaded use

				void release() {
					// Clear the members first: deleting the object can destroy this shared_ptr
					// too, if it lives inside that object, as a next pointer in a list does
					T *ptr = ptr_;
					int *ref = ref_;
					ptr_ = nullptr;
					ref_ = nullptr;

					if (ref && --*ref == 0 ) {
						delete ptr;
						delete ref;
					}
				}

				void swap(shared_ptr& other) noexcept {
					std::swap(ptr_, other.ptr_);
					std::swap(ref_, other.ref_);
				}

			public:
				// 1. Constructor
				explicit shared_ptr(T *ptr = nullptr) 
					: ptr_(ptr), ref_(ptr ? new int(1) : nullptr) {}

				// 2. Destructor
				~shared_ptr() {
					release();
				}

				// 3. Copy Constructor
				shared_ptr(const shared_ptr& sp) : ptr_(sp.ptr_), ref_(sp.ref_) {
					if (ref_) ++*ref_;
				}

				// 4. Move Constructor
				shared_ptr(shared_ptr&& sp) noexcept : ptr_(sp.ptr_), ref_(sp.ref_) {
					sp.ptr_ = nullptr;
					sp.ref_ = nullptr;
				}

				// 5. Copy Assignment
				// Copy and swap: take the new value before releasing the old one. Releasing first
				// breaks head = head->next, because that can delete the object sp lives in.
				// tmp's destructor releases the old value at the end. This also handles self-assignment.
				shared_ptr& operator=(const shared_ptr& sp) {
					shared_ptr tmp(sp);
					swap(tmp);
					return *this;
				}

				// 6. Move Assignment
				shared_ptr& operator=(shared_ptr&& sp) noexcept {
					shared_ptr tmp(std::move(sp));
					swap(tmp);
					return *this;
				}

				// 7. Nullptr Assignment
				shared_ptr& operator=(std::nullptr_t) {
					release();
					return *this;
				}

				// --- Accessors ---
				// Const correctness: accessing the data shouldn't modify the pointer
				T& operator*() const { return *ptr_; }
				T* operator->() const { return ptr_; }
				T* get() const { return ptr_; }

				size_t use_count() const { 
					return ref_ ? *ref_ : 0; 
				}

				explicit operator bool() const { return ptr_ != nullptr; }
		};

	template<typename T>
		class sharedptr {
			private:
				T *ptr_;
				std::atomic<int> *ref_; // Thread-safe atomic counter

				void release() {
					// Clear the members first, as in shared_ptr above
					T *ptr = ptr_;
					std::atomic<int> *ref = ref_;
					ptr_ = nullptr;
					ref_ = nullptr;

					// Decrement and check if we are the last owner
					if (ref && ref->fetch_sub(1, std::memory_order_acq_rel) == 1) {
						delete ptr;
						delete ref;
					}
				}

				void swap(sharedptr& other) noexcept {
					std::swap(ptr_, other.ptr_);
					std::swap(ref_, other.ref_);
				}

			public:
				// 1. Constructor
				explicit sharedptr(T *ptr = nullptr)
					: ptr_(ptr), ref_(ptr ? new std::atomic<int>(1) : nullptr) {}

				// 2. Destructor
				~sharedptr() {
					release();
				}

				// 3. Copy Constructor
				sharedptr(const sharedptr& sp) : ptr_(sp.ptr_), ref_(sp.ref_) {
					if (ref_) ref_->fetch_add(1, std::memory_order_relaxed);
				}

				// 4. Move Constructor
				sharedptr(sharedptr&& sp) noexcept : ptr_(sp.ptr_), ref_(sp.ref_) {
					sp.ptr_ = nullptr;
					sp.ref_ = nullptr;
				}

				// 5. Copy Assignment (copy and swap, as in shared_ptr above)
				sharedptr& operator=(const sharedptr& sp) {
					sharedptr tmp(sp);
					swap(tmp);
					return *this;
				}

				// 6. Move Assignment
				sharedptr& operator=(sharedptr&& sp) noexcept {
					sharedptr tmp(std::move(sp));
					swap(tmp);
					return *this;
				}

				// 7. Nullptr Assignment
				sharedptr& operator=(std::nullptr_t) {
					release();
					return *this;
				}

				// --- Accessors ---
				// Const correctness: accessing the data shouldn't modify the pointer
				T& operator*() const { return *ptr_; }
				T* operator->() const { return ptr_; }
				T* get() const { return ptr_; }

				size_t use_count() const {
					return ref_ ? ref_->load(std::memory_order_relaxed) : 0;
				}

				explicit operator bool() const { return ptr_ != nullptr; }
		};
}


void demo_own_smart_pointers() {
    std::cout << "--- Starting siva Smart Pointer Demo (2026) ---\n" << std::endl;

    // 1. Testing siva::shared_ptr (Single-Threaded Non-Atomic Version)
    {
        std::cout << "[Test 1: shared_ptr (Single-Threaded)]" << std::endl;
        siva::shared_ptr<int> sp1(new int(10));
        std::cout << "sp1 count: " << sp1.use_count() << " | Value: " << *sp1 << std::endl;

        {
            siva::shared_ptr<int> sp2 = sp1; // Copy constructor, as sp2 is new
            std::cout << "sp1 count after copy: " << sp1.use_count() << std::endl;
        } // sp2 goes out of scope here
        
        std::cout << "sp1 count after sp2 destroyed: " << sp1.use_count() << std::endl;
        
        siva::shared_ptr<int> sp3 = std::move(sp1); // Move constructor, as sp3 is new
        std::cout << "sp3 count after move: " << sp3.use_count() << " | Value: " << *sp3 << std::endl;
        if (!sp1) std::cout << "sp1 is now null after move." << std::endl;
    }
    std::cout << std::endl;

    // 2. Testing siva::sharedptr (Thread-Safe Atomic Version)
    {
        std::cout << "[Test 2: sharedptr (Atomic)]" << std::endl;
        siva::sharedptr<std::string> sptr1(new std::string("Hello Siva"));
        siva::sharedptr<std::string> sptr2 = sptr1;
        
        std::cout << "sptr2 value: " << *sptr2 << " | Count: " << sptr2.use_count() << std::endl;
        
        sptr1 = nullptr; // Resetting one pointer
        std::cout << "Count after sptr1 = nullptr: " << sptr2.use_count() << std::endl;
    }
    std::cout << std::endl;

    // 3. Testing siva::unique_ptr (Exclusive Ownership)
    {
        std::cout << "[Test 3: unique_ptr (Exclusive)]" << std::endl;
        siva::unique_ptr<double> up1(new double(3.14));
        std::cout << "up1 value: " << *up1 << std::endl;

        // siva::unique_ptr<double> up2 = up1; // ERROR: Copying not allowed
        
        siva::unique_ptr<double> up2 = std::move(up1); // Ownership Transfer
        if (!up1) std::cout << "up1 transferred ownership to up2." << std::endl;
        std::cout << "up2 value: " << *up2 << std::endl;
        
        up2.reset(new double(2.71)); // Deletes old, takes new
        std::cout << "up2 value after reset: " << *up2 << std::endl;
    }

    std::cout << "\n--- Demo Completed: All memory safely deallocated ---" << std::endl;
}


int main() {
	smart_pointer_demo();
	memory_demo();

    	demo_own_smart_pointers();

	StackOnly s;
	s.show();

	HeapOnly* h = HeapOnly::create();
	h->show();
	h->destroy();

	HeapOnlySmart::create()->show();

	return 0;
}
