/*****************************************************************************************************************************************************************
 *
 *
 * malloc vs new
 * delete vs free
 * NULL and nullptr
 * lvalue rvalue xvalue
 * smart pointers
 * overflow in new
 * Singleton with thread safe
 * valgrind
 * https://isocpp.org/wiki/faq/freestore-mgmt
 *
 * double free/delete ?
 * 2d memory allocation
 *
 *
 * References in C++:
 *
 * - References are like constant pointers that are automatically dereferenced. There is no need to use the * to dereference a reference variable.
 * - It is a new name given to an existing storage. So when you are accessing the reference, you are actually accessing that storage.
 *
 *   References								Pointers
	Reference must be initialized when it is created.	Pointers can be initialized any time.
	Once initialized, we cannot reinitialize a reference.	Pointers can be reinitialized any number of time.
	You can never have a NULL reference.			Pointers can be NULL.
	Reference is automatically dereferenced.		* is used to dereference a pointer
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
*
*   
*
*
*   		Heap <--- Stack <----- Data segment <---- Code Segment
*
*
**************************************************  rvalues, lvalues, xvalues, glvalues, and prvalues *************************************************
*
*
*
*
*
*
*
************************************************** NULL and nullptr ***********************************************************************************
*
* NULL is a “manifest constant” (a #define of C) that’s actually an integer that can be assigned to a pointer because of an implicit conversion
* nullptr is a keyword representing a value of self-defined type, that can convert into a pointer, but not into integers
*
*
*
*
* 
*
*
*
*
*/

#include <iostream>
#include <string>




char **get2dPointer(size_t row, size_t column);
void release2dPointer(char **ptr, size_t row);

class Square {
	private:
		int len;
	public:
		Square(int l = 0 ) : len(l) {};
		int area() { return len * len ;}

};

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
		~Student() { std::cout << "destructor invoked" << std::endl; }

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

		void *operator new(size_t)   {}
		void *operator new[](size_t) {}

	public:
		Employee(int id, std::string name) : id(id), name(name) { }
		~Employee() { std::cout << "destructor invoked" << std::endl; }

		void print() {
			std::cout << "id : " << id << ", name : " << name << std::endl;
		}
};

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

	return 0;
}



char **get2dPointer(size_t row, size_t column) {

	char **ptr = new char*[row];
	while ( row-- )
		ptr[row] = new char[column];
	return ptr;
}

void release2dPointer(char **ptr, size_t row) {
	
	while(row--)
		delete [] ptr[row];
	delete [] ptr;
}


