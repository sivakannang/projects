#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <array>
#include <list>
#include <algorithm>
#include <utility>
#include <cstring>

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

// template specialization
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
	template<typename T, size_t N>
	class Array
	{
		private:
			T data[N];
		public:
			size_t size() const { return N; }
			bool is_valid_idx(int idx) const { return idx >= 0 && idx < N; }

			const T& at(int idx) const {
				if ( is_valid_idx(idx))
					return data[idx];
				throw std::out_of_range("out_of_range excepion");
			}

			T& at(int idx) {
				if ( is_valid_idx(idx))
					return data[idx];
				throw std::out_of_range("out_of_range excepion");
			}

			const T& operator[] ( int idx ) const { return data[idx]; }
			T& operator[] ( int idx ) { return data[idx]; }

			T* begin() { return data; }
			T* end()   { return data + N; }
			
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
			Vector() : size_(0), capacity_(0) { }
			~Vector() { delete [] ptr_; ptr_ = nullptr; }
			size_t size() const { return size_; }
			size_t capacity() const { return capacity_; }
			bool empty() const { return size_ == 0; }
			
			T& operator[](int idx) const {
				if ( is_valid_idx(idx))
					return ptr_[idx];
				throw std::out_of_range("out_of_range excepion");
			}

			void push_back(const T& t)
			{
				if ( size_ == capacity_ )
				{
					capacity_ = ( capacity_ == 0 ) ? 1 : (capacity_ * 2);
					T *tmp = new T[ capacity_ ];
					for ( int idx = 0; idx < size_; idx++ )
						tmp[idx] = std::move(ptr_[idx]);
					delete [] ptr_;
					ptr_ = tmp;
				}

				ptr_[size_++] = t;
				//std::cout << "ptr_[" << size_-1 << "] = " << ptr_[size_-1] << std::endl;
			}

			void pop_back() { if ( size_ == 0 ) throw std::underflow_error("underflow exception"); --size_; }
			
			T* begin() { return ptr_; }
			T* end() const { return ptr_ + size_; }
	};

	
	template<typename T>
	class Stack {
		private:
			std::vector<T> vec;
		public:
			bool empty() const { return vec.size() == 0; }
			size_t size() const { return vec.size(); }
			void push(const T& t) { vec.push_back(t); }
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

	template<typename ...Args>
	struct Node;

	// Node1 for map
	template<typename K, typename V>
	struct Node<K, V> {
		K key;
		V value;
		Node<K, V>* left;
		Node<K, V>* right;
		
		Node(K key, V value, Node<K, V>* left = nullptr, Node<K, V>* right = nullptr) : key(key), value(value), left(left), right(right) {}

		bool operator == (const Node<K, V>& node) {
			return key == node.key && value == node.value;
		}
			
		friend std::ostream& operator << (std::ostream& os, const Node<K, V>* node)
		{
			os << node->key << " : " << node->value << std::endl;
			return os;
		}
	};


	// Node2 for others . Note - If we swap the place of Node1 and Node2, compiler will throw error 'redeclared with 2 template parameters'
	template<typename T>
	struct Node<T> {
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
			
			void push(const T& data)
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

			T front() const {
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
				auto& list = table[ std::hash<T>()(t) % bucketcount ];
				auto it = std::find(list.begin(), list.end(), t);
				if ( it != list.end())
					return *it;
				return nullptr;
			}

			void traverse() const {
				int idx = 0;
				std::for_each(table.begin(), table.end(), [&idx](std::list<T> list) {
					std::cout << idx++ << " : ";
					for ( T& t : list ) {
						std::cout << t << " -> "; 
					}
					std::cout << std::endl;
				} );
			}

			void erase(T t) {
				table[ std::hash<T>()(t) % bucketcount ].remove(t);
			}
	};

	template<typename K, typename V, size_t bucketcount = 10>
	class Unordered_map {
		private:
			std::array<std::list<Node<K, V>>, bucketcount> table;
		public:
			size_t bucket_count() const { return bucketcount; }
		        size_t bucket_size(int bucket_idx) const { return table[bucket_idx].size(); }

			void insert(K key, V value) {
				int idx = std::hash<K>()(key) % bucketcount ;
				table[idx].push_back(Node<K, V>(key, value));
			}

			V find(K key) const {
				int idx = std::hash<K>()(key) % bucketcount;
				for ( auto& node : table[idx] ) {
					if ( node.key == key )
						return node.value;
				}
				return nullptr;
			}

			bool erase(K key) {
				auto& list = table[ std::hash<K>()(key) % bucketcount ];
				for ( auto& node : list ) {
					if ( node.key == key ) {
						list.remove(node);
						return true;
					}
				}
				return false;
			}

			void traverse() const {
				int idx = 0;
				for ( auto& list : table ) {
					std::cout << idx++ << " : ";
					for ( auto& node : list )
						std::cout << "< " << node.key << " , " << node.value << " > ";
					std::cout << std::endl;
				}
			}
	};


	template<typename K, typename V>
	class Map {
		private:
			size_t m_size{};
			Node<K, V>* head {};


			void clear(Node<K, V>* node) {
				if ( node == nullptr )
					return;
				clear(node->left);
				clear(node->right);
				std::cout << __func__ << " -> " << node;
				delete node;
			}
			
			void dfs_in_order(Node<K, V>* node) const {
				if ( node == nullptr )
					return;
				dfs_in_order(node->left);
				std::cout << node;
				dfs_in_order(node->right);
			}

		public:
			Map() = default;

			~Map() { clear(head); }

			size_t size() const { return m_size; }
			
			void insert(K key, V value) {
				
				if ( head == nullptr ) {
					head = new Node<K, V>(key, value);
					return;
				}

				Node<K, V> *cur = head;
				Node<K, V> *parent = nullptr;
				
				while( cur ) {
					parent = cur;
					if ( key < cur->key )
						cur = cur->left;
					else if ( key > cur->key )
						cur = cur->right;
					else {
						std::cout << "duplicates are not allowed" << std::endl;
						return;
					}
				}

				if ( key < parent->key )
					parent->left = new Node<K, V>(key, value);
				else
					parent->right = new Node<K, V>(key, value);
			}
			void clear() { clear(head); }
			void dfs_in_order() { dfs_in_order(head); };

	};


	class String {
		
		private:
			size_t size_;
			char *data_;

		public:
			size_t size() const { return size_; }
			const char* c_str() const { return data_; }

			String() : size_(0) ,  data_(nullptr) {}
			~String() { delete [] data_; data_ = nullptr; }

			const char& operator[](size_t idx) const { return data_[idx]; }

			char& operator[] (size_t idx) { return data_[idx]; };

			String(const char *str) 
			{
				if ( str )
				{
					size_ = std::strlen(str);
					data_ = new char[size_ + 1];
					std::strcpy(data_, str);
				}
				else
				{
					size_ = 0;
					data_ = nullptr;
				}
			}

			String(const String& str)
			{
				size_ = str.size_;
				data_ = new char[size_ + 1];
				std::strcpy(data_, str.data_);
			}

			String& operator = (const String& str)
			{
				if ( this == &str )
				{
					return *this;
				}

				delete[] data_;
				size_ = str.size_;
				data_ = new char[size_+1];
				std::strcpy(data_, str.data_);
				return *this;
			}

			String(String&& str)
			{
				std::cout << "move constructor invoked" << std::endl;
				size_ = str.size_;
				data_ = str.data_;
				str.size_ = 0, str.data_ = nullptr;
			}

			String& operator = (String&& str)
			{
				if ( this != &str )
				{
					std::cout << "move assignment invoked" << std::endl;
					delete [] data_;
					size_ = str.size_;
					data_ = str.data_;
					str.size_ = 0, str.data_ = nullptr;
				}
				return *this;
			}

			String operator + ( const String& str)
			{
				char *ptr = new char [ size_ + str.size_ + 1];
				std::strcpy(ptr, data_);
				std::strcat(ptr, str.data_);
				String tmp(ptr);
				delete [] ptr;
				return tmp;
			}

			bool operator == ( const String& str ) const
			{
				if ( this == &str )
					return true;

				return (std::strcmp(data_, str.data_) == 0) ;
			}

			friend std::ostream& operator << (std::ostream& os, const String& str)
			{
				if ( str.c_str() )
					os << str.c_str();
				return os;
			}

			friend std::istream& operator >> (std::istream& is, String& str)
			{
				char buffer[1024]{};
				is >> buffer;
				str = String(buffer);
				return is;
			}
	};

	namespace directed_graph {

	template<typename T>
	struct Edge {
		T src;
		T dst;
	};

	// Directed Graph
	template<typename T>
	class Graph {
		private:
			std::vector<std::vector<T>> vec;
			int node_count;
		public:
			Graph(std::vector<Edge<T>> edges, int N ) : node_count(N) {
				vec.resize(node_count);
				for ( auto& edge : edges ) {
					vec[edge.src].push_back(edge.dst);
					// Uncomment the following code for undirected graph
					//vec[edge.dst].push_back(edge.src);
				}
			}

			void print() {
				for ( int i = 0; i < node_count; ++i ) {
					std::cout << i << " --> ";
					for ( auto& t : vec[i] )
						std::cout << t << " ";
					std::cout << std::endl;
				}
			}
	};
	}


	namespace directed_graph_weight {
		
		struct Edge {
			int src;
			int dst;
			int weight;
		};

		class Graph {
			private:
				std::vector<std::vector<std::pair<int, int>>> vec;
				int node_count;
			public:
				Graph(const std::vector<Edge>& edges, int N) : node_count(N) {
					vec.resize(node_count);
					for ( auto& edge : edges ) {
						vec[edge.src].push_back({edge.dst, edge.weight});
						// uncomment the following line for undirected graph
						// vec[edge.dst].push_back( {edge.src, edge.weight} );
					}
				}

				void print() {
					for ( int i = 0; i < node_count; ++i ) {
						for ( auto& pair : vec[i] )
							std::cout << "( " << i << " " << pair.first << " " << pair.second << " ) ";
						std::cout << std::endl;
					}
				}
		};
	}

}
