/*****************************************************************************************************************************************************************
 *
 *
 * malloc vs new :
 * 	Both are using allocate memory but new allows to construct the object in the memory created
 * Can we use free() for release the memory allocated using new ?
 * 	nope, both may use diffrent mechanism for allocate memory, so do not mix it
 * NULL and nullptr:
 * 	NULL is a constant in C, that can be assigned to a pointer because of implicit convertion
 * 	nullptr is a keyword that can convert into a pointer but not into integer.
 * 
 * lvalue rvalue:
 * 	We can take the address of lvalue, rvalue is temporary object, there we can't take address
 * 	int i = 5; ( here i is an lvalue and 5 is temporary object that is rvalue )
 *
 * double delete or free the memory twice ?
 * 	No effect on a nullptr but if the pointer points an valid address, it will lead to crash
 *
 * References in C++ :								Pointers
 * 	Reference must be initialized when it is created.	Pointers can be initialized any time.
 * 	Once initialized, we cannot reinitialize a reference.	Pointers can be reinitialized any number of time.
 * 	You can never have a NULL reference.			Pointers can be NULL.
 * 	Reference is automatically dereferenced.		* is used to dereference a pointer
 *
 *
 *
 * Feature                | new/delete                     | malloc/free                   
--------------------------+--------------------------------+-------------------------------
 Memory allocated from    | 'Free Store'                   | 'Heap'                        
 Returns                  | Fully typed pointer            | void*                         
 On failure               | Throws (never returns NULL)    | Returns NULL                  
 Required size            | Calculated by compiler         | Must be specified in bytes    
 Handling arrays          | Has an explicit version        | Requires manual calculations  
 Reallocating             | Not handled intuitively        | Simple (no copy constructor)  
 Call of reverse          | Implementation defined         | No                            
 Low memory cases         | Can add a new memory allocator | Not handled by user code      
 Overridable              | Yes                            | No                            
 Use of (con-)/destructor | Yes                            | No

*
*******************************************************  Memory Management in C++  ********************************************************************
*
*   		Heap <--- Stack <----- Data segment <---- Code Segment
*
************************************************** Smart pointers ***********************************************************************************
*
* Automaitc memory management
* Provides exception safety to classes and functions that handle objects with dynamic lifetime, by guaranteeing deletion on both normal exit and exception exit
*
* unique_ptr  - A smart pointer that owns a dynamically allocated resource
* shared_ptr  - a smart pointer that owns a shared dynamically allocated resource.
*             - Several std::shared_ptrs may own the same resource and an internal counter keeps track of them
* weak_ptr    - like a std::shared_ptr, but it doesn't increment the counter
*             - It use to avoid the cyclic reference
* auto_ptr    - deprecated , inefficient to handle arrays(it always calls delete in destructor, so need a custom deleter required for handle arrays for call delete[])
*             - ownership will be moved simply when passed the value to a function, C++11 introduced unique_ptr, where std::move required to move the ownership.
*
* template< class T, class Deleter = std::default_delete<T> > class unique_ptr;
*
* Deleter must be FunctionObject or lvalue reference to a FunctionObject
*
* operator= - assigns the unique pointer
* release   - Releases ownership of its stored pointer, by returning its value and replacing it with a null pointer
* reset     - Destroys the object currently managed by the unique_ptr (if any) and takes ownership of p ( if any )
* swap      - swaps the managed objects
*
* get            - returns a pointer to the managed object
* get_deleter    - returns the deleter that is used for destruction of the managed object
*
* operator bool  - checks if there is an associated managed object
* operator *     - dereferences pointer to the managed object
* operator ->    - 
* operator []    - provides indexed access to the managed array
*
* std::unique_ptr
* std::make_unique  - From C++14 for exception safety
* 
* reset() 
*
* std::unique_ptr<int>      p1(new int);
* std::unique_ptr<int[]>    p2(new int[50]);
* std::unique_ptr<Object>   p3(new Object("Lamp"));
*
* std::unique_ptr<int>      p1 = std::make_unique<int>();
* std::unique_ptr<int[]>    p2 = std::make_unique<int[]>(50);
* std::unique_ptr<Object>   p3 = std::make_unique<Object>("Lamp");
* std::unique_ptr<Object[]> p4 = std::make_unique<Object[]>(50);
*
* std::shared_ptr<int>      p1 = std::make_shared<int>();
* std::shared_ptr<Object>   p2 = std::make_shared<Object>("Lamp");
* std::shared_ptr<Student>   student(new Student[3], [](int *p) { delete [] p; });    // Until C++14
* std::shared_ptr<Student[]> student(new Student[3])                                  // From C++17
* 
* std::shared_ptr<int>  p3(new int[50], [](int *p) { delete[] p;} );    // Until C++17, for shared pointer arrays we can't use int[] ,we should specify int only
* std::shared_ptr<Student> student(new Student[3], [](Student *s) {delete[] s;});  // Until C++17, shared pointer array calls delete instead of delete[], so need to write custom deletor . Also can't use make_shared() here, as it doesn't support customized deleter here
*
* Prefer using make_unique() or make_shared() , avoid using new()
*
*  void function(std::unique_ptr<A>(new A()), std::unique_ptr<B>(new B())) { ... }
*  Suppose that new A() succeeds, but new B() throws an exception , memory silently leakes for A() , no way to clean it up.
*  But by wrapping A and B into std::make_uniques you are sure the leak will not occur
*  Prefer using make_unique() or make_shared() , avoid using new()
*
* for access array version ->    student.get()[idx] , for member access student.get()[idx].id
*
*/

#include <iostream>
#include <string>
#include <exception>




char **get2dPointer(size_t row, size_t column);
void release2dPointer(char **ptr, size_t row);

class Square {
	private:
		int len;
	public:
		Square(int l = 0 ) : len(l) {};
		int area() { return len * len ;}

};

namespace mylib {

template<typename T>
struct custom_deleter {
	void operator()(T *t) { delete t; t = nullptr; }
};

template<typename T, typename Deleter = custom_deleter<T>>
class Unique_ptr {
	private:
		T *ptr_ = nullptr;
		Deleter operator_delete;
	public:
		Unique_ptr() : ptr_(nullptr) {}
		Unique_ptr(std::nullptr_t) : ptr_(nullptr) {}
		explicit Unique_ptr(T* ptr) noexcept : ptr_(std::move(ptr)) {}
		~Unique_ptr() { std::cout << __func__ << std::endl; operator_delete(ptr_);}

		Unique_ptr(const Unique_ptr<T>& unique_ptr) = delete;
		Unique_ptr<T>& operator = (const Unique_ptr<T>& unique_ptr) = delete;

		Unique_ptr(Unique_ptr<T>&& unique_ptr) noexcept : ptr_(std::move(unique_ptr->ptr_)) { unique_ptr->ptr_ = nullptr; }
		Unique_ptr<T>& operator = (Unique_ptr<T>&& unique_ptr) noexcept {
			if ( this == unique_ptr ) return *this;
			delete ptr_;
			ptr_ = std::move(unique_ptr->ptr_);
			unique_ptr->ptr_ = nullptr;
			return *this;
		}

		T* operator ->() const { return ptr_; }
		T& operator *() const { return *ptr_; }

		T* get() const noexcept { return ptr_; }
		explicit operator bool() const noexcept { return ptr_; }

		T* release()
		{
			//return std::exchange(ptr_, nullptr); // from c++14
			//std::exchange is a setter and returning the old value
			//int z = std::exchange(x, y);
			//x is assigned the value of y,
			//z is assigned the value that x had initially.
			T *result = nullptr;
			std::swap(result, ptr_);
			return result;
		}

		void reset(T *ptr) noexcept(false)
		{
			if ( ptr == nullptr )
				throw std::invalid_argument("An invalid pointer was passed, resources will not be swapped");
			operator_delete(ptr_);
			std::swap(ptr_, ptr);
		}
};

template<typename T, typename Deleter = custom_deleter<T>>
class Shared_ptr {
	private:
		T *ptr_;
		int *count;
		Deleter operator_delete;

	public:
		explicit Shared_ptr(T *ptr = nullptr) : ptr_(ptr), count(new int(0))
		{
			if ( ptr_ )
				++(*count);
		}

		~Shared_ptr()
		{
			if ( count != nullptr )
			{
				--(*count);
				if ( *count == 0 ) {
					operator_delete(ptr_);
					delete count;
				}
			}
		}

		Shared_ptr(const Shared_ptr<T>& shared_ptr)
		{
			ptr_ = shared_ptr->ptr_;
			count = shared_ptr->count;
			++(*count);
		}

		Shared_ptr<T>& operator = (const Shared_ptr<T>& shared_ptr)
		{
			/*if ( count != nullptr )
			{
				--(*count);
				if ( *count == 0 )
					delete ptr_;
			}*/

			ptr_ = shared_ptr->ptr_;
			count = shared_ptr->count;
			++(*count);
			return *this;
		}

		T& operator * () const { return *ptr_; }
		T* operator -> () const { return ptr_; }
		
		T* get() const noexcept { return ptr_; }
		explicit operator bool() const noexcept { return ptr_; }

		size_t use_count() const { return (count == nullptr ) ? 0 : *count; }

		void reset()
		{
			if ( count != nullptr )
			{
				--(*count);
				if ( count == 0 )
				{
					operator_delete(ptr_);
					delete count;
				}
				ptr_ = nullptr;
				count = nullptr;
			}
		}
};
}


// https://medium.com/analytics-vidhya/c-shared-ptr-and-how-to-write-your-own-d0d385c118ad  - rewrite using this, below one is wrong
namespace my {
template <typename T>
class shared_ptr {

	private:
		T *ptr;
		int refCount;

	public:

		shared_ptr(T *p = nullptr ) : ptr(p), refCount(0) {
			std::cout << "my::shared_ptr -> Constructor invoked, refCount = " << ++refCount << std::endl;
		}

		shared_ptr( const shared_ptr& sp)  {
			ptr = sp.ptr;
			refCount = sp.refCount + 1;
			std::cout << "my::shared_ptr -> Copy Constructor invoked, refCount = " << ++refCount << std::endl;
		}

		shared_ptr& operator= (const shared_ptr& sp) {
			ptr = sp.ptr;
			refCount = sp.refCount + 1;
			std::cout << "my::shared_ptr -> Copy Assignment invoked, refCount = " << ++refCount << std::endl;
		}

		~shared_ptr() {
			--refCount;
			std::cout << "my::shared_ptr -> Destructor invoked, refCount = " << refCount << std::endl;
			if ( refCount == 0) {
				delete ptr;
				std::cout << "my::shared_ptr -> Resource released, refCount = " << refCount << std::endl;
			}
		}

		T& operator *()  { std::cout << "my::shared_ptr -> Dereference invoked" << std::endl; return *ptr; }
		T* operator ->() { std::cout << "my::shared_ptr -> Get address invoked" << std::endl;return ptr; }
		int use_count() const { return refCount; }
		T *get() const { return ptr; }
		void reset( T *p = nullptr) {
			if ( refCount > 0 ) {
				delete ptr;
				refCount = 0;
			}

			if ( p ) {
				ptr = p;
				refCount = 1;
			}
		}


};
}

class Student {		// A class which always create instance in heap

	private:
		int id;
		std::string name;
		Student(int id, std::string name) : id(id), name(name) { }
		~Student() { std::cout << __func__ << std::endl; }

	public:
		static Student *createStudent(int id, std::string name) {
			return new Student(id, name);
		}
		void print() {
			std::cout << "id : " << id << ", name : " << name << std::endl;
		}
		void Delete() { delete this;}
};

class Employee {		// A class which always create instance in stack
	
	private:
		int id;
		std::string name;

	public:
		void *operator new(size_t)   = delete; // delete this function or move it to private section
		void *operator new[](size_t) = delete; // delete this function or move it to private section

		Employee(int id, std::string name) : id(id), name(name) { }
		~Employee() { std::cout << "destructor invoked" << std::endl; }

		void print() {
			std::cout << "id : " << id << ", name : " << name << std::endl;
		}
};

void my_smart_ptr_test()
{
	//mylib::Unique_ptr<Student> student = Student::createStudent(46, "siva");
}
int main() {

	//Student student(2, "ganesh");                -> compilation error
	Student *student = Student::createStudent(4, "siva");
	student->print();
	student->Delete();

	Employee employee(5, "kannan");
	employee.print();
	//Employee *emp = new Employee(6, "six");  -> compilation error
	
	char **ptr = get2dPointer(20, 100);
	release2dPointer(ptr, 20);

	my::shared_ptr<Square> sPtr(new Square(4));
	std::cout << "Square area(4) = " << sPtr->area() << std::endl;

	my_smart_ptr_test();

	return 0;
}



char **get2dPointer(size_t row, size_t column) {

	char **ptr = new char*[row];
	while ( row-- )
		ptr[row] = new char[column];
	return ptr;

	// how to access n th value array using base pointer array
	// *( p +  n )
	// how to access value of 2d pointer array using base pointer
	// *(*( p + row ) + col)
}

void release2dPointer(char **ptr, size_t row) {
	
	while(row--)
		delete [] ptr[row];
	delete [] ptr;
}


