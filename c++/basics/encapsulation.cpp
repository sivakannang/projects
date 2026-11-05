// g++ -std=c++17 encapsulation.cpp -o encapsulation

/******************* Encapsulation *************************
*
* - A class is a user-defined blueprint that encapsulates data and functions.
* - An object is an instance of a class.
* - Encapsulation means bundling data (variables) and methods (functions) that operate on the data into a single unit (class).
* - Access specifiers control visibility:
*     * `private`: accessible only within the class
*     * `protected`: accessible in the class and derived classes
*     * `public`: accessible from outside the class
* - Encapsulation ensures data hiding and abstraction by restricting direct access to internal variables.
*
| Member             | Auto-generated when…                    | Common blockers / notes                                                     |
| ------------------ | --------------------------------------- | --------------------------------------------------------------------------- |
| Default ctor       | No constructors declared                | Any user ctor exists → write `= default` if needed                          |
| Destructor         | No user destructor                      | Declaring one **suppresses auto-move**                                      |
| Copy ctor / assign | No copy/move declared                   | Members/bases not copyable ⇒ **deleted**                                    |
| Move ctor / assign | No copy/move **and** no user destructor | Members/bases not movable ⇒ **deleted**; user dtor/copy/move **suppresses** |

Rule of Five / Zero :
  If you define any of: copy/move/dtor ⇒ likely define all that matter.
  If your type owns nothing special ⇒ define none (Rule of Zero)

***************************** Interview Questions *******************************
*
* 1. What is a class in C++?
*    - A user-defined type that groups data and functions together.
*
* 2. What is an object?
*    - A concrete instance of a class with its own memory and data.
*
* 3. What is encapsulation?
*    - The concept of wrapping data and methods into a single unit.
*
* 4. What is the default access specifier for class members?
*    - `private` for classes, `public` for structs.
*
* 5. How is data hiding achieved in C++?
*    - By declaring class members `private` or `protected`.
*
* 6. Can functions be encapsulated in C++?
*    - Yes, both data and methods are part of encapsulation in C++.
*
* ************************************************************************/

#include <iomanip>
#include <iostream>
#include <string>

// Encapsulation Example
// Money is kept as whole paise: double can't represent most decimal amounts exactly, so its errors add up
class BankAccount {
	private:
		std::string accountHolder;
		long long balancePaise;

	public:
		BankAccount(const std::string& name, long long initialPaise)
			: accountHolder(name), balancePaise(initialPaise) {}

		void deposit(long long paise) {
			if (paise > 0)
				balancePaise += paise;
		}

		// Returns false if the amount is invalid or the balance is too low
		bool withdraw(long long paise) {
			if (paise <= 0 || paise > balancePaise)
				return false;

			balancePaise -= paise;
			return true;
		}

		void printSummary() const {
			std::cout << "Account Holder: " << accountHolder << "\n";
			std::cout << "Balance: Rs. " << balancePaise / 100 << "." << std::setw(2) << std::setfill('0') << balancePaise % 100 << "\n";
		}
};

int main() {
	BankAccount acc("Siva", 1000000);
	acc.deposit(250000);
	if (!acc.withdraw(120000))
		std::cout << "Withdrawal refused\n";
	acc.printSummary();
	return 0;
}

