
#include <iostream>

template<typename T, int _size>
class ARRAY {
	private:
		T *m_ptr{nullptr};                    // {} member initialization support, from C++11
		int m_size{0};
		bool isValidIndex(int idx) const { return idx >= 0 && idx < m_size ; }
	public:
		ARRAY() : m_size(_size) {
			m_ptr = new T[m_size]{};      // {} - initialize to 0, supports from C++11
		}
		~ARRAY() { delete[] m_ptr; }
		bool empty() const { return m_size == 0;}
		int size()   const { return m_size;}

		T& operator [](int idx) {
			
			if ( isValidIndex(idx))
				return m_ptr[idx];
			throw std::out_of_range("out_of_range exception");
		}

};

void test_array(); 

template<typename T>
class STACK {

	private:
		std::vector<T> v;
		int _maxsize;
		
	public:
		STACK(int size=5) : _maxsize(size) {}

		bool push(T t);
		bool pop();
		T top() const;

		int size() const { return v.size(); }
		bool isEmpty() const { return size() == 0; }
		bool isFull()  const { return size() == _maxsize;}
		void traverse() const;
};



template<typename T>
class Node {
	public:
		T data;
		Node<T> *right;
		Node<T> *left;
		Node(T t, Node<T> *rp = nullptr, Node<T> *lp = nullptr) : data(t), right(rp), left(lp) {}

};

template<typename T>
class QUEUE {

	private:
		Node<T> *frontNode;
		Node<T> *rearNode;
		int max_size;
		int cur_size;

		int size() const { return cur_size; }
		bool isEmpty() const { return size() == 0; }
		bool isFull()  const { return size() == max_size;}
		Node<T> *getHead() const { return frontNode; }

	public:
		QUEUE(int size = 5) : frontNode(nullptr), rearNode(nullptr), cur_size(0), max_size(size) {}
		~QUEUE() {
			Node<T> *tmp;
			while(frontNode) {
				tmp = frontNode;
				frontNode = frontNode->right;
				delete tmp;
			}
		}

		bool push(T t);
		bool pop();
		T top() const;
		void traverse() const;
};


class Employee {
	private:
		long rollno;
		std::string name;
		double salary;
	public:
		//Employee(long rollno = 0, std::string name = nullptr, double salary = 0) : _rollno(rollno), _name(name), _salary(salary) {} - crashing
		Employee(long rollno = 0, std::string name = "", double salary = 0) : rollno(rollno), name(name), salary(salary) {}
		

		bool operator < (const Employee &employee) const { return this->rollno < employee.rollno; }
		bool operator > (const Employee &employee) const { return this->rollno > employee.rollno; }

		friend std::ostream &operator<<( std::ostream &os, const Employee &employee)  {
			os << "rollno : " << employee.rollno << " , name : " << employee.name << " , salary : " << employee.salary << std::endl;
			return os;
		}

		friend std::istream& operator >> ( std::istream& is, Employee& employee) {
			std::cout << "Enter roll number : ";
			is >> employee.rollno;
			std::cout << std::endl;
			std::cout << "Enter  name       : ";
			is >> employee.name;
			std::cout << std::endl;
			std::cout << "Enter salary      : ";
			is >> employee.salary;
			std::cout << std::endl;
			return is;
		}
};

template <typename T>
class Tree {
	protected:
		Node<T> *root;
		Node<T> *head() { return root; }
		Node<T> *_insert(Node<T> *head, T &data);
		void _dfs_in_order(Node<T> *head);

	public:
		Tree(Node<T> *node = nullptr) : root(node) {}
		Node<T> *insert(T data) { root = _insert(head(), data); dfs_in_order(); }
		void dfs_in_order()     { _dfs_in_order(head()); }
		void dfs_pre_order();
		void dfs_post_order();
		void bfs_traverse();
};

template<typename T>
class BST : public Tree<T> {
};

