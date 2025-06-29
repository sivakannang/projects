#include <iostream>
#include <cstring>
#include <memory>

class String
{
	private:
		size_t size_;
		std::unique_ptr<char[]> data_;

	public:
		size_t size() const { return size_; }
		const char* c_str() { return data_.get(); }
		const char& operator[] (size_t idx) const { return data_.get()[idx]; }
		char& operator[] (size_t idx) { return data_.get()[idx]; }
		
		String() : size_(0), data_(nullptr) {}
		
		String(const char *str )
		{
			if ( str )
			{
				size_ = std::strlen(str);
				data_ = std::make_unique<char []>( size_ + 1 );
				std::strcpy(data_.get(), str);
			}
			else
			{
				size_ = 0;
				data_ = nullptr;
			}
		}

		String(const String& str)
		{
			size_ = str.size();
			if ( size_ > 0 )
			{
				data_ = std::make_unique<char []>(size_ + 1);
				std::strcpy(data_.get(), str.data_.get());
			}
			else
			{
				data_ = nullptr;
			}
		}

		String(String&& str) noexcept : size_(str.size_), data_(std::move(str.data_))
		{
			std::cout << "move constructor invoked" << std::endl;
			str.size_ = 0;
		}

		String& operator = ( const String& str )
		{
			size_ = str.size_;
			if ( size_ == 0 )
			{
				data_ = nullptr;
			}
			else
			{
				data_ = std::make_unique<char []>(size_ + 1);
				std::strcpy(data_.get(), str.data_.get());
			}
			return *this;
		}

		String& operator= (String&& str) noexcept
		{
			std::cout << "move assignment invoked" << std::endl;
			if ( this != &str )
			{
				size_ = str.size_;
				data_ = std::move(str.data_);
				str.size_ = 0;
			}
			return *this;
		}

		String operator + ( const String& str )
		{
			String tmp;
			tmp.size_ = size_ + str.size_;
			if ( tmp.size_ == 0 )
			{
				tmp.data_ = nullptr;
			}
			else
			{
				tmp.data_ = std::make_unique<char []>(tmp.size_ + 1);
				if ( this->data_ ) std::strcpy(tmp.data_.get(), data_.get());
				if (  str.data_ ) std::strcat(tmp.data_.get(), str.data_.get());
			}
			return tmp;
		}

		bool operator == ( const String& str )
		{
			if ( !data_ && !str.data_ )  return true;
			
			if ( !data_ || !str.data_ )  return false;

			return std::strcmp(data_.get(), str.data_.get()); 
		}

		friend std::ostream& operator << ( std::ostream& os, const String& str )
		{
			if ( str.data_ )
			{
				std::cout << str.data_.get() << std::endl;
			}
			return os;
		}
};

void string_test()
{
	std::cout << "<<---------- " << __func__ << "---------->>" << std::endl;

	String s1;                     // default constructor
	String s2("siva");             // argument constructor
	String s3("kannan");           // argument constructor
	String s4(s2);                 // copy constructor
	String s5 = s3;                // copy constructor
	String s6 = s2 + s3;           // operator + and copy constructor
	

	std::cout << "s1 : " << s1 << std::endl;
	std::cout << "s2 : " << s2 << std::endl;
	std::cout << "s3 : " << s3 << std::endl;
	std::cout << "s4 : " << s4 << std::endl;
	std::cout << "s5 : " << s5 << std::endl;
	std::cout << "s6 : " << s6 << std::endl;
	
	s6 = s2 + s3 + s6;                  // operator + and copy assignment
	std::cout << "s6 : " << s6 << std::endl;
	s3 = s2 + s3 + s2 + s3;
	std::cout << "s3 : " << s3 << std::endl;

	if ( s3 == s6 )
	{
		std::cout << "s3 and s6 are same " << std::endl;
	}
	
	if ( !(s3 == s2) )
	{
		std::cout << "s3 and s2 are not same " << std::endl;
	}
	
	if ( s3 == s3 )
	{
		std::cout << "s3 and s3 are same " << std::endl;
	}

	std::cout << s3[0] << std::endl;  // const char& operator[] (size_t idx) const
	s3[0] = 'I';                      //       char& operator[] (size_t idx)
	std::cout << s3 << std::endl;

	String s7 = std::move(String("test"));  // std::move will create rvalue reference, move construtor will invoke
	std::cout << "s7 " << s7 << std::endl;
	s7 = String("done with the test");              // std::move will create rvalue reference, move assginment will invoke
	std::cout << "s7 " << s7 << std::endl;
	
	String s10;
	String s11;
	s11 = s10;
	std::cout << "s11 " << s11 << std::endl;
	
}


int main()
{
	string_test();
	return 0;
}
