
/* 
---------------------------------------------------------
   Memory Layout and Types in C++
---------------------------------------------------------

✅ What is Memory Layout?

When you run a C++ program, the operating system divides its memory into regions:

+---------------------+
|   Stack             |
+---------------------+
|   Heap              |
+---------------------+
|   BSS Segment       |
+---------------------+
|   Data Segment      |
+---------------------+
|   Text Segment      |
+---------------------+

Memory Layout:

- stack -> local_var, automatically managed(allocated/deallocated), fast but limit in size 
- heap  -> dynamic memory allocation using new/delete, must be managed by programmer
        -> heap_var - pointer on stack, points to heap
- data  -> initialized global and static variablesi
        -> string literals will be stored in read-only data segment
- bss   -> uninitialized global & static variables
- Code  -> Text segment

---------------------------------------------------------
✅ Typical Address Order

On most systems:

- Text → lowest addresses
- Data / BSS → above text
- Heap → grows upward
- Stack → grows downward

Heap and stack grow toward each other, generally heap grows upward and stach grows downward

---------------------------------------------------------
✅ Stack Overflow vs Heap Overflow

- Stack overflow:
    - infinite recursion
- Heap overflow:
    - writing past allocated memory
    - can cause corruption
---------------------------------------------------------
✅ sizeof and Alignment

Data alignment affects structure size.

Example:

    struct MyStruct {
        char a;
        int b;
    };

May occupy 8 bytes:
- 1 byte for char
- 3 bytes padding
- 4 bytes for int

Check size:

    std::cout << sizeof(MyStruct) << std::endl;

---------------------------------------------------------
   Dynamic Memory Allocation in C++
---------------------------------------------------------

// C++ supports manual dynamic memory management using:
// - new/delete (preferred in C++)
// - malloc/free (from C, use with care in C++)

// Notes:
// - `new` allocates memory and calls constructors.
// - `delete` frees memory and calls destructors.
// - Always pair new-delete and new[]-delete[].
// - Prefer smart pointers for exception-safe management (C++11+).

Questions:

// 1. What is dynamic memory allocation?
//    - Allocating memory during runtime from the heap.

// 2. What is the difference between new/delete and malloc/free?
//    - new/delete call constructors/destructors; malloc/free do not.

// 3. What happens if delete is not called?
//    - Memory leak occurs.

// 4. What are smart pointers used for?
//    - To automatically manage heap memory and avoid leaks.

// 5. What is the syntax for array allocation using new?
//    - int* arr = new int[10];
// 6. What happens if you call delete twice on the same pointer?
//    - Undefined behavior; may crash or corrupt memory.

// 7. What happens if you call delete on a nullptr?
//    - It is safe and has no effect.
// 8. How can we restrict a class to be created only on the stack?
//    - Make operator new and delete private or delete them to prevent heap allocation.

// 9. How can we restrict a class to be created only on the heap?
//    - Make constructors private and provide static create() method that returns a pointer.



---------------------------------------------------------
   Smart Pointers in C++
---------------------------------------------------------

// Smart pointers automate memory management by releasing memory when no longer needed.
// Introduced in C++11 and available in <memory> header.

// Types:
// - std::unique_ptr: sole ownership, cannot be copied.
// - std::shared_ptr: reference-counted shared ownership.
// - std::weak_ptr: non-owning reference to a shared_ptr.

// Notes:
// - Avoids memory leaks and dangling pointers.
// - Prefer smart pointers over raw new/delete.
// - Use make_unique and make_shared for safe construction.

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
// - release()     -> releases ownership and returns raw pointer
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

// std::make_unique(), std::make_shared() - From C++14 for exception safety

// std::unique_ptr<int>      p1(new int);
// std::unique_ptr<int[]>    p2(new int[50]);
// std::unique_ptr<Object>   p3(new Object("Lamp"));

// std::unique_ptr<int>      p1 = std::make_unique<int>();
// std::unique_ptr<int[]>    p2 = std::make_unique<int[]>(50);
// std::unique_ptr<Object>   p3 = std::make_unique<Object>("Lamp");
// std::unique_ptr<Object[]> p4 = std::make_unique<Object[]>(50);

// std::shared_ptr<int>      p1 = std::make_shared<int>();
// std::shared_ptr<Object>   p2 = std::make_shared<Object>("Lamp");
// std::shared_ptr<Student>   student(new Student[3], [](int *p) { delete [] p; });    // Until C++14
// std::shared_ptr<Student[]> student(new Student[3])                                  // From C++17
 
// std::shared_ptr<int>  p3(new int[50], [](int *p) { delete[] p;} );   // Until C++17, for shared pointer arrays we can't use int[] ,we should specify int only
// std::shared_ptr<Student> student(new Student[3], [](Student *s) {delete[] s;});
        - Until C++17, shared pointer array calls delete instead of delete[], so need to write custom deletor . Also can't use make_shared() here, as it doesn't support customized deleter here

// Prefer using make_unique() or make_shared() and avoid using new()    // make_unique() is exception safety introduced in c++14 

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
    static int static_var = 30; // Stored in static segment

    std::cout << "Stack: " << stack_var << "\n";
    std::cout << "Heap: " << *heap_var << "\n";
    std::cout << "Static: " << static_var << "\n";

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

    std::unique_ptr<int> uptr = std::make_unique<int>(100);
    std::cout << "unique_ptr: " << *uptr << std::endl;

    std::shared_ptr<int> sptr1 = std::make_shared<int>(200);
    std::shared_ptr<int> sptr2 = sptr1;
    std::cout << "shared_ptr use_count: " << sptr1.use_count() << std::endl;
    std::cout << "shared_ptr: " << *sptr1 << std::endl;

    std::weak_ptr<int> wptr = sptr1;
    if (auto locked = wptr.lock()) {
        std::cout << "weak_ptr locked value: " << *locked << std::endl;
    } else {
        std::cout << "weak_ptr expired" << std::endl;
    }
    wptr.reset();
}

template <typename T>
class MyUniquePtr {
    T* ptr;
public:
    explicit MyUniquePtr(T* p = nullptr) : ptr(p) {}
    ~MyUniquePtr() { delete ptr; }

    MyUniquePtr(const MyUniquePtr&) = delete;
    MyUniquePtr& operator=(const MyUniquePtr&) = delete;

    MyUniquePtr(MyUniquePtr&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }

    MyUniquePtr& operator=(MyUniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
};

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
