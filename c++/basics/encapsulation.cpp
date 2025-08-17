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

#include <iostream>
#include <string>

// Encapsulation Example
class BankAccount {
	private:
		std::string accountHolder;
		double balance;

	public:
		BankAccount(const std::string& name, double initialBalance) {
			accountHolder = name;
			balance = initialBalance;
		}

		void deposit(double amount) {
			if (amount > 0)
				balance += amount;
		}

		void withdraw(double amount) {
			if (amount > 0 && amount <= balance)
				balance -= amount;
		}

		void printSummary() const {
			std::cout << "Account Holder: " << accountHolder << "\n";
			std::cout << "Balance: Rs. " << balance << "\n";
		}
};

int main() {
	BankAccount acc("Siva", 10000);
	acc.deposit(2500);
	acc.withdraw(1200);
	acc.printSummary();
	return 0;
}

