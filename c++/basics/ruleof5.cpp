#include <iostream>
#include <string>
#include <cstring>


class Address {

	public:
		std::string city;

		Address(const std::string& c = "") : city(c) {}
		//~Address() = default;
		//Address(const Address&) = default;
		//Address& operator=(const Address&) = default;
		//Address(Address&&) noexcept = default;
		//Address& operator=(Address&&) noexcept = default;

		void print() const {
			std::cout << "city: " << city << std::endl;
		}
};

class User1 {

	private:
		char *name;
		Address address;

	public:
		User1(const char *n = nullptr, const Address& addr = Address()) : name(nullptr), address(addr)
	{
		if ( n )
		{
			name = new char[std::strlen(n)+1];
			std::strcpy(name, n);
		}

	}

		~User1()
		{
			delete [] name;
		}

		User1(const User1& u) : name(nullptr), address(u.address)
	{
		if ( u.name )
		{
			name = new char[std::strlen(u.name)+1];
			std::strcpy(name, u.name);
		}
	}

		User1(User1&& u) noexcept : name(u.name), address(std::move(u.address))
		{
			u.name = nullptr;
		}

		User1& operator = (const User1& u)
		{
			if ( this != &u )
			{
				delete [] name;
				name = nullptr;
				if ( u.name )
				{
					name = new char[std::strlen(u.name)+1];
					std::strcpy(name, u.name);
				}
				address = u.address;
			}
			return *this;
		}

		User1& operator = (User1&& u) noexcept 
		{
			if ( this != &u )
			{
				delete [] name;
				name = u.name;
				u.name = nullptr;
				address = std::move(u.address);
			}
			return *this;
		}
};

class User2 {

	private:
		char *name;
		Address *address;

	public:
		User2(const char *n = nullptr, const Address* addr = nullptr ) : name(nullptr), address(nullptr)
	{
		if ( n ) {
			name = new char[std::strlen(n)+1];
			std::strcpy(name, n);
		}

		if ( addr )  { address = new Address(*addr); }
	}

		~User2()
		{
			delete [] name;
			delete address;
		}

		User2(const User2& u) : name(nullptr), address(nullptr)
	{
		if ( u.name ) {
			name = new char[std::strlen(u.name)+1];
			std::strcpy(name, u.name);
		}

		if ( u.address )  { address = new Address(*u.address); }
	}

		User2& operator = (const User2& u )
		{
			if ( this != &u )
			{
				delete [] name; name = nullptr;
				if ( u.name ) {
					name = new char[std::strlen(u.name)+1];
					std::strcpy(name, u.name);
				}
				delete address; address = nullptr;
				if ( u.address ) { address = new Address(*u.address); }
			}
			return *this;
		}

		User2(User2&& u) noexcept : name(u.name), address(u.address)
		{
			u.name = nullptr;
			u.address = nullptr;
		}

		User2& operator = (User2&& u) noexcept
		{
			if ( this != &u )
			{
				delete [] name;
				name = u.name;
				u.name = nullptr;

				delete address;
				address = u.address;
				u.address = nullptr;
			}
			return *this;
		}


};


class User3 {
	private:
		char*    name;
		Address& address;  // reference, must be initialized in constructor

	public:
		// 1. Constructor
		User3(const char* n, Address& addr) : name(nullptr), address(addr) 
		{
			if (n) {
				name = new char[std::strlen(n) + 1];
				std::strcpy(name, n);
			}
		}

		// 2. Destructor
		~User3() {
			delete[] name;
		}

		// 3. Copy Constructor
		User3(const User3& other) : name(nullptr), address(other.address) // bind to same Address
		{
			if (other.name) {
				name = new char[std::strlen(other.name) + 1];
				std::strcpy(name, other.name);
			}
		}

		// 4. Copy Assignment (deleted — cannot rebind reference)
		User3& operator=(const User3&) = delete;

		// 5. Move Constructor
		User3(User3&& other) noexcept : name(other.name), address(other.address) // bind to same Address
		{
			other.name = nullptr;
		}

		// 6. Move Assignment (deleted — cannot rebind reference)
		User3& operator=(User3&&) = delete;

		void print() const {
			std::cout << "Name: " << (name ? name : "(null)") << '\n';
			address.print();
		}
};


int main()
{
	Address address("cts");

	User1 user1("kanna", address);

	return 0;
}
