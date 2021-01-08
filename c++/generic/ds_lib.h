
#include <iostream>
#include <exception>

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

			void pop_back() { if ( size_ == 0 ) throw std::out_of_range("underflow exception"); --size_; }
			
			T* begin() const { return &ptr_[0]; }
			T* end() const { return &ptr_[size_]; }
	};

}
