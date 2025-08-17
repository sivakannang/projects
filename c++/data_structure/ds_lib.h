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
#include <memory>
#include <optional>


namespace dsa
{


	template<typename T, size_t N>
		class Array
		{

			T data_[N];

			public:
			size_t size() const noexcept { return N; }

			// unchecked (like std::vector::operator[])
			T& operator[](size_t i) noexcept { return data_[i]; }
			const T& operator[](std::size_t i) const noexcept { return data_[i]; }

			// checked access
			T& at(std::size_t i) {
				if (i >= N) throw std::out_of_range("Array::at");
				return data_[i];
			}
			const T& at(std::size_t i) const {
				if (i >= N) throw std::out_of_range("Array::at");
				return data_[i];
			}

			// iterators
			T*       begin()        { return data_; }
			T*       end()          { return data_ + N; }
			const T* begin()  const { return data_; }
			const T* end()    const { return data_ + N; }

			std::reverse_iterator<T*>  rbegin()   { return std::reverse_iterator<T*>(end()); }
			std::reverse_iterator<T*>  rend()     { return std::reverse_iterator<T*>(begin()); }
		};


	template<typename T>
		class Vector {
			private:
				std::size_t size_{0};
				std::size_t capacity_{0};
				T*          data_{nullptr};

			public:
				Vector() = default;
				~Vector() { delete[] data_; data_ = nullptr; }

				// capacity/size
				std::size_t size()     const { return size_; }
				std::size_t capacity() const { return capacity_; }
				bool        empty()    const { return size_ == 0; }

				// element access (single non-const versions, with inline checks)
				T& operator[](std::size_t idx) { return data_[idx]; }
				T& at(std::size_t idx) {
					if (idx >= size_) throw std::out_of_range("Vector::at out_of_range");
					return data_[idx];
				}

				// modifiers
				void push_back(const T& t) {
					if (size_ == capacity_) {
						std::size_t new_cap = capacity_ ? capacity_ * 2 : 1;
						T* tmp = new T[new_cap];                 // requires T default-constructible & assignable
						for (std::size_t i = 0; i < size_; ++i)
							tmp[i] = std::move(data_[i]);
						delete[] data_;
						data_ = tmp;
						capacity_ = new_cap;
					}
					data_[size_++] = t;
				}

				void pop_back() {
					if (size_ == 0) throw std::underflow_error("Vector::pop_back underflow");
					--size_;
				}

				// iterators (non-const only)
				T* begin() { return data_; }
				T* end()   { return data_ + size_; }
		};


	template<typename T>
		class Stack {
			private:
				std::vector<T> vec;
			public:
				bool empty() const { return vec.empty(); }
				size_t size() const { return vec.size(); }

				void push(const T& t) { vec.push_back(t); }
				void push(T&& t) { vec.push_back(std::move(t)); }

				void pop() {
					if ( empty() )
						throw std::underflow_error("underflow_error");
					vec.pop_back();
				}
				T& top() {
					if ( empty() )
						throw std::underflow_error("underflow_error");
					return vec.back();
				}

				template<class... Args>
					T& emplace(Args&&... args) {
						vec.emplace_back(std::forward<Args>(args)...);
						return vec.back();
					}
		};


	template<typename T>
		class Queue {
			private:
				struct Node {
					T     data;
					Node* next;

					template<class U>
						explicit Node(U&& v)  : data(std::forward<U>(v)), next(nullptr) {}
				};

				std::size_t size_{0};
				Node*       front_node{nullptr};
				Node*       rear_node{nullptr};

			public:
				Queue() = default;
				~Queue() { clear(); }

				Queue(const Queue&)            = delete;
				Queue& operator=(const Queue&) = delete;

				bool        empty() const noexcept { return size_ == 0; }
				std::size_t size()  const noexcept { return size_; }

				void clear() noexcept {
					while (front_node) {
						Node* tmp = front_node->next;
						delete front_node;
						front_node = tmp;
					}
					rear_node = nullptr;
					size_ = 0;
				}

				// Single push handles lvalues, rvalues, and convertible types
				template<class U>
					void push(U&& value) {
						Node* new_node = new Node(std::forward<U>(value));
						if (rear_node) rear_node->next = new_node;
						else           front_node      = new_node;
						rear_node = new_node;
						++size_;
					}

				T& front() {
					if (empty()) throw std::underflow_error("Queue::front on empty");
					return front_node->data;
				}

				void pop() {
					if (empty()) throw std::underflow_error("Queue::pop on empty");
					Node* tmp = front_node;
					front_node = front_node->next;
					delete tmp;
					--size_;
					if (!front_node) rear_node = nullptr;
				}
		};



	template<typename T>
		class List {
			private:
				struct Node {
					T     data;
					Node* prev;
					Node* next;

					//Node(const T& v, Node* p=nullptr, Node* n=nullptr) : data(v), prev(p), next(n) {}
					//Node(T&& v, Node* p=nullptr, Node* n=nullptr) : data(std::move(v)), prev(p), next(n) {}

					// handles both lvalue and rvalue
					template<class U>
						Node(U&& v, Node *p = nullptr, Node *n = nullptr) : data(std::forward<U>(v)), prev(p), next(n) {}

				};

				std::size_t size_{0};
				Node*       head_{nullptr};
				Node*       tail_{nullptr};

			public:
				List() = default;
				~List() { clear(); }

				// No copy/move to keep it minimal
				List(const List&)            = delete;
				List& operator=(const List&) = delete;

				bool        empty() const noexcept { return size_ == 0; }
				std::size_t size()  const noexcept { return size_; }

				void clear() noexcept {
					Node* cur = head_;
					while (cur) {
						Node* nxt = cur->next;
						delete cur;
						cur = nxt;
					}
					head_ = tail_ = nullptr;
					size_ = 0;
				}

				// handles lvalue only
				void push_back(const T& v) {
					Node* newNode = new Node(v, tail_, nullptr);
					if (tail_) tail_->next = newNode; else head_ = newNode;
					tail_ = newNode;
					++size_;
				}

				// handles rvalue only
				void push_back(T&& v) {
					Node* newNode = new Node(std::move(v), tail_, nullptr);
					if (tail_) tail_->next = newNode; else head_ = newNode;
					tail_ = newNode;
					++size_;
				}

				// handles both lvalue and rvalue
				template<class U>
					void push_front(U&& v) {
						Node* newNode = new Node(std::forward<U>(v), nullptr, head_);
						if (head_) head_->prev = newNode; else tail_ = newNode;
						head_ = newNode;
						++size_;
					}

				void pop_back() {
					if (empty()) throw std::underflow_error("List::pop_back on empty");
					if (size_ == 1) { delete tail_; head_ = tail_ = nullptr; size_ = 0; return; }
					Node* old = tail_;
					tail_ = tail_->prev;
					tail_->next = nullptr;
					delete old;
					--size_;
				}

				void pop_front() {
					if (empty()) throw std::underflow_error("List::pop_front on empty");
					if (size_ == 1) { delete head_; head_ = tail_ = nullptr; size_ = 0; return; }
					Node* old = head_;
					head_ = head_->next;
					head_->prev = nullptr;
					delete old;
					--size_;
				}

				T& front() {
					if (empty()) throw std::underflow_error("List::front on empty");
					return head_->data;
				}
				T& back() {
					if (empty()) throw std::underflow_error("List::back on empty");
					return tail_->data;
				}
		};


	template<typename K, typename V, std::size_t N = 10>
		class HashTable {
			static_assert(N > 0, "HashTable: N must be > 0");

			std::size_t count{0};
			std::vector<std::list<std::pair<K, V>>> table;

			std::size_t index_for(const K& k) const { return std::hash<K>{}(k) % N; }

			public:
			HashTable() : table(N) {}
			std::size_t size() const { return count; }

			// Perfect-forwarding insert (handles lvalues/rvalues/convertibles)
			template<class KK, class VV>
				void insert(KK&& k, VV&& v) {
					const auto idx = index_for(static_cast<const K&>(k));
					auto& bucket = table[idx];
					for (auto& kv : bucket) {
						if (kv.first == k) { kv.second = std::forward<VV>(v); return; }
					}
					bucket.emplace_back(std::forward<KK>(k), std::forward<VV>(v));
					++count;
				}

			bool remove(const K& k) {
				const auto idx = index_for(k);
				auto& bucket = table[idx];
				for (auto it = bucket.begin(); it != bucket.end(); ++it) {
					if (it->first == k) { bucket.erase(it); --count; return true; }
				}
				return false;
			}

			// 1) Value copy
			std::optional<V> find(const K& k) const {
				const auto idx = index_for(k);
				const auto& bucket = table[idx];
				for (const auto& kv : bucket)
					if (kv.first == k) return kv.second;  // copy out
				return std::nullopt;
			}

			// 2) Reference (no copy)
			std::optional<std::reference_wrapper<V>> find_reference(const K& k) {
				const auto idx = index_for(k);
				auto& bucket = table[idx];
				for (auto& kv : bucket)
					if (kv.first == k) return kv.second;  // wraps V& in reference_wrapper
				return std::nullopt;
			}

			// 3) Pointer (no copy)
			V* find_pointer(const K& k) {
				const auto idx = index_for(k);
				auto& bucket = table[idx];
				for (auto& kv : bucket)
					if (kv.first == k) return &kv.second;
				return nullptr;
			}

			void print() const {
				std::size_t b = 0;
				for (const auto& bucket : table) {
					std::cout << "bucket[" << b++ << "]: ";
					for (const auto& kv : bucket)
						std::cout << "{" << kv.first << "," << kv.second << "} ";
					std::cout << "\n";
				}
			}
		};



	template<typename K, typename V>
		class Map {
			private:
				struct Node {
					K     key;
					V     value;
					Node* left  = nullptr;
					Node* right = nullptr;

					template<class KK, class VV>
						Node(KK&& k, VV&& v) : key(std::forward<KK>(k)), value(std::forward<VV>(v)) {}
				};

				std::size_t m_size_{0};
				Node*       root_{nullptr};

				static void clear_node(Node* n) noexcept {
					if (!n) return;
					clear_node(n->left);
					clear_node(n->right);
					delete n;
				}
				static void dfs_in_order_node(const Node* n) {
					if (!n) return;
					dfs_in_order_node(n->left);
					std::cout << n->key << ":" << n->value << " ";
					dfs_in_order_node(n->right);
				}

			public:
				Map() = default;
				~Map() { clear(); }

				Map(const Map&)            = delete;
				Map& operator=(const Map&) = delete;

				Map(Map&& other) noexcept : m_size_(other.m_size_), root_(other.root_) {
					other.m_size_ = 0; other.root_ = nullptr;
				}
				Map& operator=(Map&& other) noexcept {
					if (this != &other) {
						clear();
						m_size_ = other.m_size_;
						root_   = other.root_;
						other.m_size_ = 0; other.root_ = nullptr;
					}
					return *this;
				}

				std::size_t size()  const noexcept { return m_size_; }
				bool        empty() const noexcept { return m_size_ == 0; }

				void clear() noexcept {
					clear_node(root_);
					root_ = nullptr;
					m_size_ = 0;
				}

				template<class KK, class VV>
					bool insert(KK&& key, VV&& value) {
						if (!root_) {
							root_ = new Node(std::forward<KK>(key), std::forward<VV>(value));
							++m_size_;
							return true;
						}
						Node* cur = root_;
						Node* parent = nullptr;
						while (cur) {
							parent = cur;
							if (key < cur->key)               cur = cur->left;
							else if (cur->key < key)          cur = cur->right;
							else { cur->value = std::forward<VV>(value); return false; }
						}
						Node* n = new Node(std::forward<KK>(key), std::forward<VV>(value));
						if (key < parent->key) parent->left = n; else parent->right = n;
						++m_size_;
						return true;
					}

				// Non-const find only (returns nullptr if not found)
				V* find(const K& key) {
					Node* cur = root_;
					while (cur) {
						if (key < cur->key)         cur = cur->left;
						else if (cur->key < key)    cur = cur->right;
						else                        return &cur->value;
					}
					return nullptr;
				}

				void dfs_in_order() const {
					dfs_in_order_node(root_);
					std::cout << '\n';
				}
		};


	template <class T>
		class RingBuffer {
			public:
				explicit RingBuffer(std::size_t capacity) : buf_(capacity), head_(0), size_(0) {
					if (capacity == 0) throw std::invalid_argument("capacity must be > 0");
				}

				// Accepts lvalues/rvalues and any type convertible to T
				template <class U>
					bool push(U&& v) {
						if (full()) return false;
						buf_[tail_index()].emplace(std::forward<U>(v)); // constructs T from U
						++size_;
						return true;
					}

				// Pop; returns empty optional if buffer is empty
				std::optional<T> pop() {
					if (empty()) return std::nullopt;
					std::optional<T> out = std::move(buf_[head_]);
					buf_[head_].reset();
					head_ = (head_ + 1) % buf_.size();
					--size_;
					return out;
				}

				T& front() const { ensure_not_empty(); return *buf_[head_]; }
				T& back()  const { ensure_not_empty(); return *buf_[idx_back()]; }

				// Queries
				bool empty()    const noexcept { return size_ == 0; }
				bool full()     const noexcept { return size_ == buf_.size(); }
				std::size_t size()     const noexcept { return size_; }
				std::size_t capacity() const noexcept { return buf_.size(); }

				void clear() noexcept {
					for (auto& cell : buf_) cell.reset();
					head_ = 0; size_ = 0;
				}

			private:
				std::size_t tail_index() const noexcept { return (head_ + size_) % buf_.size(); }
				std::size_t idx_back()   const noexcept { return (head_ + size_ - 1) % buf_.size(); }
				void ensure_not_empty()  const {
					if (empty()) throw std::out_of_range("RingBuffer: empty");
				}

				std::vector<std::optional<T>> buf_;
				std::size_t head_;
				std::size_t size_;
		};


	class String {
		private:
			size_t size_{0};
			char*  data_{nullptr};

		public:
			size_t size() const { return size_; }
			const char* c_str() const { return data_ ? data_ : ""; } // never nullptr

			String() = default;

			~String() { delete[] data_; data_ = nullptr; }

			String(const char* str) : size_(0), data_(nullptr) {
				if (str) {
					size_ = std::strlen(str);
					data_ = new char[size_ + 1]{};
					if (size_) std::memcpy(data_, str, size_);
				}
			}

			String(const String& other) : size_(other.size_), data_(nullptr) {
				if (size_) {
					data_ = new char[size_ + 1]{};
					std::memcpy(data_, other.data_, size_);
				}
			}

			String& operator=(const String& other) {
				if (this != &other)
				{
					if ( other.size_ == 0 ) { delete[] data_ , data_ = nullptr, size_ = 0; return *this; }

					char *tmp = new char[other.size_ + 1]();
					std::memcpy(tmp, other.data_, other.size_);
					delete[] data_;
					data_ = tmp;
					size_ = other.size_;
				}
				return *this;
			}

			String(String&& other) noexcept : size_(other.size_), data_(other.data_) {
				other.size_ = 0; other.data_ = nullptr;
			}

			String& operator=(String&& other) noexcept {
				if (this != &other) {
					delete[] data_;
					size_ = other.size_;
					data_ = other.data_;
					other.size_ = 0; other.data_ = nullptr;
				}
				return *this;
			}

			const char& operator[](size_t idx) const { return data_[idx]; }
			char&       operator[](size_t idx)       { return data_[idx]; }
			char& at(size_t idx) {
				if (idx >= size_) throw std::out_of_range("String::at out_of_range");
				return data_[idx];
			}

			String operator+(const String& rhs) const {
				if (size_ == 0) return rhs;          // cheap fast-paths
				if (rhs.size_ == 0) return *this;

				String out;
				out.size_ = size_ + rhs.size_;
				out.data_ = new char[out.size_ + 1]{};
				std::memcpy(out.data_,          data_,     size_);
				std::memcpy(out.data_ + size_,  rhs.data_, rhs.size_);
				return out;
			}

			bool operator==(const String& rhs) const {
				if (this == &rhs) return true;
				if (size_ != rhs.size_) return false;
				return std::memcmp(c_str(), rhs.c_str(), size_) == 0;
			}

			bool operator!=(const String& rhs) const { return !(*this == rhs); }

			void swap(String& other) noexcept {
				std::swap(size_, other.size_);
				std::swap(data_, other.data_);
			}

			friend std::ostream& operator<<(std::ostream& os, const String& str) {
				return os << str.c_str();            // safe: never nullptr
			}

			friend std::istream& operator>>(std::istream& is, String& str) {
				std::string tmp;
				if (is >> tmp) str = String(tmp.c_str());
				return is;
			}
	};


	class MyString {
		private:
			size_t                   size_{0};
			std::unique_ptr<char[]>  data_{nullptr};

		public:
			size_t size() const { return size_; }
			const char* c_str() const { return data_ ? data_.get() : ""; } // never nullptr

			MyString() = default;
			~MyString() = default; // unique_ptr cleans up

			// As unique_ptr is moveable default version suits well
			// std::move is just a cast to rvalue (T&&). It doesn’t move anything by itself.
			// so remember std::move = copy for built in types like int, so it won't reset other.size_ to 0.
			MyString(MyString&&) noexcept = default;
			MyString& operator=(MyString&&) noexcept = default;

			MyString(const char* str) : size_(0), data_(nullptr) {
				if (str) {
					size_ = std::strlen(str);
					data_ = std::make_unique<char[]>(size_ + 1);
					if (size_) std::memcpy(data_.get(), str, size_);
					data_[size_] = '\0';
				}
			}

			MyString(const MyString& other) : size_(other.size_), data_(nullptr) {
				if (size_) {
					data_ = std::make_unique<char[]>(size_ + 1);
					std::memcpy(data_.get(), other.data_.get(), size_);
					data_[size_] = '\0';
				}
			}

			MyString& operator=(const MyString& other) {
				if (this != &other) {
					if (other.size_ == 0) { data_.reset(); size_ = 0; return *this; }

					auto tmp = std::make_unique<char[]>(other.size_ + 1);
					std::memcpy(tmp.get(), other.data_.get(), other.size_);
					tmp[other.size_] = '\0';

					data_ = std::move(tmp);
					size_ = other.size_;
				}
				return *this;
			}


			// If you provided only the non-const overload, indexing a const object wouldn’t compile.
			// If you provided only the const overload, you couldn’t assign through operator[]
			const char& operator[](size_t idx) const { return data_[idx]; }
			char&       operator[](size_t idx)       { return data_[idx]; }

			char& at(size_t idx) {
				if (idx >= size_) throw std::out_of_range("MyString::at out_of_range");
				return data_[idx];
			}

			MyString operator+(const MyString& rhs) const {
				if (size_ == 0) return rhs;
				if (rhs.size_ == 0) return *this;

				MyString out;
				out.size_ = size_ + rhs.size_;
				out.data_ = std::make_unique<char[]>(out.size_ + 1);
				std::memcpy(out.data_.get(),           data_.get(),     size_);
				std::memcpy(out.data_.get() + size_,   rhs.data_.get(), rhs.size_);
				out.data_[out.size_] = '\0';
				return out;
			}

			bool operator==(const MyString& rhs) const {
				if (this == &rhs) return true;
				if (size_ != rhs.size_) return false;
				return std::memcmp(c_str(), rhs.c_str(), size_) == 0;
			}

			bool operator!=(const MyString& rhs) const { return !(*this == rhs); }

			void swap(MyString& other) noexcept {
				std::swap(size_, other.size_);
				std::swap(data_, other.data_);
			}

			friend std::ostream& operator<<(std::ostream& os, const MyString& s) {
				return os << s.c_str();
			}

			friend std::istream& operator>>(std::istream& is, MyString& s) {
				std::string tmp;
				if (is >> tmp) s = MyString(tmp.c_str());
				return is;
			}
	};



	class Trie {
		struct Node {
			Node* child[256]{};   // one slot per ASCII byte
			int   pass = 0;       // #words passing through this node
			int   end  = 0;       // #words ending at this node
		};

		Node*        root_ = new Node();
		std::size_t  size_ = 0;

		static void destroy(Node* n) {
			if (!n) return;
			for (Node* c : n->child) if (c) destroy(c);
			delete n;
		}

		public:
		Trie() = default;
		~Trie() { destroy(root_); }

		Trie(const Trie&)            = delete;
		Trie& operator=(const Trie&) = delete;

		std::size_t size()  const noexcept { return size_; }
		bool        empty() const noexcept { return size_ == 0; }

		void insert(std::string_view s) {
			Node* cur = root_;
			for (unsigned char uc : s) {
				if (!cur->child[uc]) cur->child[uc] = new Node();
				cur = cur->child[uc];
				++cur->pass;
			}
			++cur->end;
			++size_;
		}

		bool contains(std::string_view s) const {
			const Node* cur = root_;
			for (unsigned char uc : s) {
				cur = cur->child[uc];
				if (!cur) return false;
			}
			return cur->end > 0;
		}

		bool starts_with(std::string_view prefix) const {
			const Node* cur = root_;
			for (unsigned char uc : prefix) {
				cur = cur->child[uc];
				if (!cur) return false;
			}
			return true;
		}

		// Stub: would remove one occurrence of 's' and prune trailing nodes no longer used.
		bool erase(std::string_view /*s*/) {
			return false; // not implemented
		}
	};



	template<typename T, class Compare = std::less<T>>
		class PriorityQueue {
			std::vector<T> data_;
			Compare        comp_;  // for max-heap: comp_(a,b) == (a < b)

			void sift_up(std::size_t i) {
				while (i > 0) {
					std::size_t p = (i - 1) / 2;
					if (!comp_(data_[p], data_[i])) break; // parent >= child
					std::swap(data_[p], data_[i]);
					i = p;
				}
			}
			void sift_down(std::size_t i) {
				const std::size_t n = data_.size();
				for (;;) {
					std::size_t best = i, l = 2*i + 1, r = l + 1;
					if (l < n && comp_(data_[best], data_[l])) best = l;
					if (r < n && comp_(data_[best], data_[r])) best = r;
					if (best == i) break;
					std::swap(data_[i], data_[best]);
					i = best;
				}
			}

			public:
			PriorityQueue() = default;

			bool        empty() const noexcept { return data_.empty(); }
			std::size_t size()  const noexcept { return data_.size(); }

			const T& top() const {
				if (empty()) throw std::underflow_error("PriorityQueue::top on empty");
				return data_.front();
			}

			template<class U>
				void push(U&& v) {
					data_.emplace_back(std::forward<U>(v));   // constructs T in-place
					sift_up(data_.size() - 1);
				}


			template<class... Args>
				void emplace(Args&&... args) {
					data_.emplace_back(std::forward<Args>(args)...);
					sift_up(data_.size() - 1);
				}

			void pop() {
				if (empty()) throw std::underflow_error("PriorityQueue::pop on empty");
				std::swap(data_.front(), data_.back());
				data_.pop_back();
				if (!empty()) sift_down(0);
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
