/********************************** PImpl Idiom in C++ **************************
 *
 * PImpl - Pointer to IMPLementation
 *
 * The main point of the pimpl idiom is to hide the implementation, especially in member variables and private methods.
 *
 * How to implement :
 *
 * 1. Create a separate class ( or struct ) for implementation
 * 2. Put all private members from the header to that class.
 * 3. Define an Implementation class ( Impl ) in the header file.
 * 4. In the header file create a forward declaration (a pointer) , pointing at the implementation class
 * 5. Define a destructor and a copy / assignment operators
 *
 * Advantages of PImpl:
 *
 * Binary Compatibility: The binary interface is independent of the private fields. Making changes to the implementation would not break the dependent code.
 *
 * Compilation time: Compilation time drops due to the fact that only the implementation file needs to be rebuilt instead of every client recompiling his file.
 *
 * Data Hiding: Can easily hide certain internal details such as implementation techniques and other libraries used to implement the public interface.
 *
 *
 ******************************************************************************/


#include <memory>

class API {
	public:
		API();
		~API();
		void execute();
	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
};
