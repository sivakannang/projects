#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <array>
#include <list>
#include <algorithm>

class Employee
{
	private:
		long rollno;
		std::string name;
		double salary;
	public:
		Employee(long rollno, std::string name, double salary) : rollno(rollno), name(name), salary(salary) {}

		long get_rollno() const { return rollno; }
		std::string get_name() const { return name; }
		double get_salary() const { return salary; }

		bool operator <  (const Employee &employee) const { return this->rollno < employee.rollno; }
		bool operator >  (const Employee &employee) const { return this->rollno > employee.rollno; }
		bool operator == (const Employee &e       ) const { return rollno == e.rollno && name == e.name && salary == e.salary; } 

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

namespace std {
	template<>
	struct hash<Employee> {
		size_t operator() (const Employee& emp) const {

			return std::hash<long>()(emp.get_rollno()) ^ std::hash<std::string>()(emp.get_name()) ^ std::hash<double>()(emp.get_salary());
		}
	};
}

namespace dsa
{
	template<typename T, size_t _size>
	class Array
	{
		private:
			size_t size_;
			T *ptr_;
		public:
			Array() : size_(_size) { ptr_ = new T[size_]{}; }
			~Array() { delete [] ptr_; ptr_ = nullptr; }
			size_t size() const { return size_; }
			bool is_valid_idx(int idx) const { return idx >= 0 && idx < size_; }

			T& operator[](int idx) const {
				if ( is_valid_idx(idx))
					return ptr_[idx];
				throw std::out_of_range("out_of_range excepion");
			}

			T* begin() const { return &ptr_[0]; }
			T* end() const { return &ptr_[size_]; }
	};


	template<typename T>
	class Vector
	{
		private:
			size_t size_;
			size_t capacity_;
			T *ptr_;
			
			bool is_valid_idx(int idx) const { return idx >= 0 && idx < size_; }
		public:
			Vector() : size_(0), capacity_(1) { ptr_ = new T[capacity_] {}; }
			~Vector() { delete [] ptr_; ptr_ = nullptr; }
			size_t size() const { return size_; }
			size_t capacity() const { return capacity_; }
			
			T& operator[](int idx) const {
				if ( is_valid_idx(idx))
					return ptr_[idx];
				throw std::out_of_range("out_of_range excepion");
			}

			void push_back(T t)
			{
				if ( size_ == capacity_ )
				{
					int idx = 0;
					capacity_ = capacity_ * 2;
					T *tmp = new T[ capacity_ ];
					for ( int idx = 0; idx < size_; idx++ )
						tmp[idx] = ptr_[idx];
					delete [] ptr_;
					ptr_ = tmp;
				}

				ptr_[size_++] = t;
				std::cout << "ptr_[" << size_-1 << "] = " << ptr_[size_-1] << std::endl;
			}

			void pop_back() { if ( size_ == 0 ) throw std::underflow_error("underflow exception"); --size_; }
			
			T* begin() const { return &ptr_[0]; }
			T* end() const { return &ptr_[size_]; }
	};

	
	template<typename T>
	class Stack {
		private:
			std::vector<T> vec;
		public:
			bool empty() const { return vec.size() == 0; }
			size_t size() const { return vec.size(); }
			void push(T t) { vec.push_back(t); }
			void pop() {
				if ( empty() )
					throw std::underflow_error("underflow_error");
				vec.pop_back();
			}
			T top() const {
				if ( empty() )
					throw std::underflow_error("underflow_error");
				return vec[vec.size() - 1];
			}
	};

	template<typename T>
	struct Node {
		T data;
		Node<T>* next;    // for queue
		Node<T>* left;    // for list or bst
		Node<T>* right;   // for list or bst

		Node(T data, Node<T>* next = nullptr, Node<T>* left = nullptr, Node<T>* right = nullptr) : data(data), next(next), left(left), right(right) {}
	};

	template<typename T>
	class Queue {
		private:
			int size_;
			Node<T>* front_node;
			Node<T>* rear_node;
		public:
			
			Queue() : size_(0) { front_node = rear_node = nullptr; }
			void clear() {
				while (front_node)
				{
					Node<T>* tmp = front_node;
					front_node = front_node->next;
					std::cout << __func__ << " : " << tmp->data << std::endl;
					delete tmp;
				}
				front_node = rear_node = nullptr;
			}
			~Queue() { clear(); }

			bool empty() const { return size_ == 0; }
			size_t size() const { return size_; }
			
			void push(T data)
			{
				Node<T>* new_node = new Node<T>(data);
				if ( front_node == nullptr )
					front_node = rear_node = new_node;
				else
				{
					rear_node->next = new_node;
					rear_node = new_node;
				}
				++size_;
			}

			T top() const {
				if ( empty() )
					throw std::underflow_error("underflow_error");
				return front_node->data;
			}

			void pop() {
				if ( empty() )
					throw std::underflow_error("underflow_error");
				Node<T>* tmp = front_node;
				front_node = front_node->next;
				delete tmp;
				--size_;
			}
	};

	template<typename T>
	class List {
		private:
			size_t size_;
			Node<T>* head;
			Node<T>* tail;
		public:
			bool empty() const { return size_ == 0; }
			size_t size() const { return size_; }
			
			List() : size_(0) { head = tail = nullptr; }
			void clear() {
				while ( head ) {
					Node<T>* tmp = head;
					head = head->right;
					std::cout << __func__ << " : " << tmp->data << std::endl;
					delete tmp;
				}
				head = tail = nullptr;
				size_ = 0;
			}
			~List() { clear(); }

			void push_back(T data) {
				Node<T>* new_node = new Node<T>(data);
				if ( tail ) {
					new_node->left = tail;
					tail->right = new_node;
					tail = new_node;
				}
				else
					head = tail = new_node;
				++size_;
			}

			void pop_back() {
				if ( tail == nullptr )
					throw std::underflow_error("underflow_error");
				else if ( tail->left == nullptr ) // single node
					clear();
				else {
					Node<T>* tmp = tail;
					tail = tail->left;
					tail->right = nullptr;
					delete tmp;
				}
				--size_;
			}

			void push_front(T data) {
				Node<T>* new_node = new Node<T>(data);
				if ( head ) {
					head->left = new_node;
					new_node->right = head;
					head = new_node;
				}
				else
					head = tail = new_node;
				++size_;
			}

			void pop_front() {
				if ( size_ == 0 )
					throw std::underflow_error("underflow_error");

				else if ( size_ == 1 ) // single node
					clear();
				else {
					Node<T>* tmp = head;
					head = head->right;
					head->left = nullptr;
					delete tmp;
				}
				--size_;
			}

	};

	template<typename T, size_t bucketcount = 10>
	class Hash {
		private:
			std::array<std::list<T>, bucketcount> table;

		public:
			size_t hash(T& data) const {
				//std::hash<T> hash_idx;
				//return hash_idx(data) % bucketcount;
				return std::hash<T>()(data) % bucketcount;
			}

			size_t bucket_count() const { return bucketcount; }
		        size_t bucket_size(int bucket_idx) const { return table[bucket_idx].size(); }	

			void insert(T t) {
				int idx = hash(t);
				table[idx].push_back(t);
			}

			T find(T t) const {
				int idx = hash(t);
				std::list<T>& list = table[idx];

				return list.find(t);
				//return (data != list.end()) ? data : nullptr;
			}

			void traverse() const {
				int idx = 0;
				std::for_each(table.begin(), table.end(), [&idx](std::list<T> list) {
					std::cout << idx++ << " <<--->> ";
					for ( T& t : list ) {
						std::cout << t << " --> "; 
					}
					std::cout << std::endl;
				} );
			}

			void erase(T t) {
				int idx = hash(t);
				std::list<T>& list = table[idx];
				return list.remove(t);
			}
	};

}
