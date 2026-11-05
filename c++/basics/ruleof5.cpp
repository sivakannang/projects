// g++ -std=c++17 ruleof5.cpp -o ruleof5
//
// Rule of Five: a class that manages a resource itself usually needs all five special members.
// User1, User2 and User3 show this with raw pointers and a reference member.
//
// Rule of Zero: prefer members that manage themselves, and then write none of them. See UserZero below.

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>


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

		void print() const {
			std::cout << "Name: " << (name ? name : "(null)") << '\n';
			address.print();
		}
};

class User2 {

	private:
		char *name;
		Address *address;

		// If a constructor throws, the destructor doesn't run, because the object never existed.
		// So name would leak if creating the Address threw, and it must be freed here.
		void copyAddress(const Address* addr)
		{
			if ( !addr )
				return;

			try {
				address = new Address(*addr);
			} catch (...) {
				delete [] name;
				throw;
			}
		}

	public:
		User2(const char *n = nullptr, const Address* addr = nullptr ) : name(nullptr), address(nullptr)
	{
		if ( n ) {
			name = new char[std::strlen(n)+1];
			std::strcpy(name, n);
		}

		copyAddress(addr);
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

		copyAddress(u.address);
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

		void print() const {
			std::cout << "Name: " << (name ? name : "(null)") << '\n';
			if ( address ) address->print();
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


// Rule of Zero: the same data as User1 and User2, with nothing to write. The compiler-generated
// members copy, move and destroy correctly, and a throwing constructor can't leak.
// unique_ptr makes this class move-only. Add a copy constructor only if copies are needed.
class UserZero {
	private:
		std::string name;
		std::unique_ptr<Address> address;

	public:
		explicit UserZero(std::string n = "", const Address& addr = Address())
			: name(std::move(n)), address(std::make_unique<Address>(addr)) {}

		void print() const {
			std::cout << "Name: " << name << '\n';
			if ( address ) address->print();
		}
};


int main()
{
	Address address("cts");

	User1 user1("kanna", address);
	User1 user1Copy(user1);                 // Copy constructor
	User1 user1Moved(std::move(user1));     // Move constructor
	user1 = user1Copy;                      // Copy assignment
	user1Copy = std::move(user1Moved);      // Move assignment
	user1.print();

	User2 user2("siva", &address);
	User2 user2Copy(user2);
	User2 user2Moved(std::move(user2));
	user2 = user2Copy;
	user2Copy = std::move(user2Moved);
	user2.print();

	User3 user3("ganesh", address);
	User3 user3Copy(user3);
	User3 user3Moved(std::move(user3));
	user3Moved.print();

	UserZero zero("zero", address);
	UserZero zeroMoved(std::move(zero));
	zeroMoved.print();

	return 0;
}
