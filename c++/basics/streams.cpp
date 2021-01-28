
/***************************************************************************************************************************************
 *
 *                                           Editor or IDE
 *                                                 |
 *                                                 |     <------- Source codes ( *.cpp, *.c ) , headers ( *.h )
 *                                                 |
 *                                           Preprocessor
 *                                                 |
 *                                                 |     <------- ( *.i , *.ii ) Included files, comments stripped, macro substitution
 *                                                 |
 *                                             Assembler     
 *                                                 |
 *                                                 |     <------- *.s
 *                                                 |
 *                                              Compiler
 *                                                 |
 *                                                 |     <------- Object codes ( *.obj, *.o )
 *                                                 |
 *           Static libraries ( .lib, .a ) ----> Linker
 *                                                 |
 *                                                 |     <------- Executable code ( *.exe )
 *                                                 |
 *        Shared libraries ( *.dll, *.so ) ----> Loader
 *                                                 |
 *                                                 |
 *                                                 |
 *                               Input ----->     CPU   -------> Output
 *
 * 
 *
 *  Build  -> Proprocessor, Assembler, Compiler, Linker
 *  Run    -> Loader, CPU
 *
 *
 *  g++ -save-temps streams.cpp           [ Save the files which will be create during compilation )
 *  readelf -a streams.o                  [ read elf objects ]
 *
 *
 * ------------------------------------------------- why c++ ---------------------------------------------------------------------------
 *  OOPS
 *   * Encapsulation    -> binding properties and actions in to a class
 *   * Abstraction      -> Hide complex things , expose only the required things 
 *   * Inheritence      -> Code reuse
 *   * Polymorphism     -> Different behaviours ( virtual , override), which allows us to redefine the way something works
 *
 *  Overloading
 *  Generic program using templates
 *  Strong TypeCheck or TypeSafety ( short smallNumber = 1234567890;   // ERROR: this value is outside the range of short )
 *  Exception handling and inline functions
 *  STL
 *
 *
 *  http://www.ntu.edu.sg/home/ehchua/programming/#Cpp
 *
 *  ------------------------------------------------ printf vs cout --------------------------------------------------------------------
 *  1. Speed       -> printf is faster than cout ( number of instruction in assembly code)
 *  2. type safety -> cout is type safety, compiler takes care
 *  3. call type   -> operator << can be overloaded easily in our classes in cout
 *  4. Format      -> Format is easier in printf than cout
 *  5. Return      -> printf returns the actual character count while cout does not have a return value. operator << overloads for stream insertion has a return value, it returns a reference to cout i.e they return the stream that was provided as an operand. This is what allows chaining of insertion operations (and for input streams, extraction operations using >>).
 *
 *  ------------------------------------------------ cout cerr clog diff ---------------------------------------------------------------
 *  cout -> stdout stream - buffered
 *  clog -> stderr stream - buffered
 *  cerr -> stderr stream - unbuffered
 *
 *  ------------------------------------------------ Flushing the Output Buffer --------------------------------------------------------
 *  
 *  endl & '\n'    -> both works as 'enter key' but endl flushes the output buffer as well
 *
 *
 *  You can flush the output buffer via:

	flush member function or manipulator:
	
	cout.flush();                  // using member function
 
	cout << "hello" << flush;      // using manipulators
	cout << endl;

	
	cout << "Enter a number: ";
	int number;
	cin >> number;                 // output buffer is flushed when input is pending
 *
 * 
 * ------------------------------------------------- Flushing input buffer -------------------------------------------------------------
 *
 *  istream & ignore (int n = 1, int delim = EOF)
 *
 *  cin.ignore(numeric_limits<streamsize>::max());        // Ignore to the end-of-file
 *  cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Ignore to the end-of-line
 *
 * When an input error is made, the stream "breaks," cin returns false, and the program stops.
 * But what if we want to recover from the error, rather than have the program stop? There are two steps to recovering from an error:
 * 1. Clear the error with cin.clear().
 * 2. Remove the incorrect characters from the stream. One way to do this is with cin.ignore().
 *
 * * ----------------------------------------------- Unformatted Input/Output Functions ( istream and ostream ) ------------------------
 *
 * put(), get() and getline()
 *
 * // Examples
 * cout.put('A');
 * cout.put('A').put('p').put('p').put('\n');
 * cout.put(65);
 *
 * int get ();
 * istream & get (char & c);
 * istream & get     (char * cstr, streamsize n, char delim = '\n');  //  It will keep    the delim char in the input stream, append null in the end
 * istream & getline (char * cstr, streamsize n, char delim = '\n');  //  It will discard the delim char in the input stream, append null in the end
 *
 * istream & read (char * buf, streamsize n)                          //  used for binary input, doesn't append null in the end
 * ostream & write (const char * buf, streamsize n)                   //  read() and write() for binary file, which read/write raw bytes without interpreting the bytes
 * streamsize gcount() const -> Return the number of character extracted by the last unformatted input operation by get(), getline(), ignore() or read(), ...
 *
 *
 * char peek ();                 // Returns the next character in the input buffer without extracting it
 * istream & putback (char c);   // Insert the character back to the input buffer
 *
 * ------------------------------------------------- States of Streams -----------------------------------------------------------------
 *
 *  The stream superclass ios_base maintains a data member to describe the states of the stream, which is a bitmask of the type iostate. The flags are:
 *
 *	eofbit    : set when an input operation reaches end-of-file.
 *	failbit   : The last input operation failed to read the expected characters or output operation failed to write the expected characters.
 *		    e.g., getline() reads n characters without reaching delimiter character.
 *	badbit    : serious error due to failure of an IO operation (e.g. file read/write error) or stream buffer.
 *      goodbit   : Absence of above error with value of 0.
 *
 *  These flags are defined as public static members in ios_base.
 *  They can be accessed directly via ios_base::failbit or via subclasses such as cin::failbit, ios::failbit.
 *  However, it is more convenience to use these public member functions of ios class:
 *
 *	good()    : returns true if goodbit is set (i.e., no error).
 *	eof()     : returns true if eofbit is set.
 *	fail()    : returns true if failbit or badbit is set.
 *	bad()     : returns true if badbit is set.
 *	clear()   : clear eofbit, failbit and badbit.
 * 
 * 
 *--------------------------------- Formatting Input/Output via Manipulators in <iomanip> and <iostream> -----------------------------------
 *
 * C++ provides a set of manipulators to perform input and output formatting:
 *
 *    <iomanip> header : setw(), setprecision(), setbas(), setfill().
 *    <iostream> header: left|right|internal, boolalpha|noboolalpha, showpos|noshowpos, uppercase|nouppercase, skipws|noskipws etc.
 *    <iostream> header: Floating-point Format fixed|scientific, setprecision, showpoint|noshowpoint
 *    <iostream> header: Integral Number Base  dec|oct|hex, setbase|showbase|noshowbase
 *    <iostream> header: Flush output after each insertion operation.  unitbuf|nounibuf
 *
 *-------------------------------------------------------------- File Streams -------------------------------------------------------------------
*
* iostream     -> istream, ostream             -> cin cout cerr clog
* fstream      -> ifstream, ofstream
* stringstream -> istringstream, ostringstream
*
* open(), close() and is_open(), clear()
*
* File modes are defined as static public member in ios_base superclass
*
* void open(const char *filename, ios::openmode mode)
* clear() ->  clearing any fail bits first (such as the end-of-file bit)
*
* ios::in    - open file for input operation
* ios::out   - open file for output operation
* ios::app   - output appends at the end of the file.
* ios::trunc - truncate the file and discard old contents.
* ios::binary- for binary (raw byte) IO operation, instead of character-based.
* ios::ate   - position the file pointer "at the end" for input/output.
*
*
* Binary file, read() and write()
* We need to use read() and write() member functions for binary file (file mode of ios::binary), which read/write raw bytes without interpreting the bytes.
*
* istream & read (char * buf, streamsize n)                          //  used for binary input, doesn't append null in the end
* ostream & write (const char * buf, streamsize n)                   //  read() and write() for binary file, which read/write raw bytes without interpreting the bytes
*
* seekg() -> get the input  file pointer address -> get()
* seekp() -> get the output file pointer address -> put()
*
* istream & seekg (streampos pos)                         -> absolute position relative to beginning
* istream & seekg (streamoff offset, ios::seekdir dir)    -> with offset (positive or negative) relative to seekdir
* streampos tellg ()                                      -> Returns current position of input pointer
*
* ostream & seekp (streampos pos)                         -> absolute position relative to beginning
* ostream & seekp (streamoff offset, ios::seekdir dir)    -> with offset (positive or negative) relative to seekdir
* streampos tellp ()                                      -> Returns current position of output pointer
*
* dir values
* 
* ios_base::beg (offset from the beginning of the stream's buffer).
* ios_base::cur (offset from the current position in the stream's buffer).
* ios_base::end (offset from the end of the stream's buffer).
*
*------------------------------------------------------------------------------------------------------------------------------------------

*------------------------------------------------------------ String Streams --------------------------------------------------------------
*
* explicit ostringstream (ios::openmode mode = ios::out);                      // default with empty string
* explicit ostringstream (const string & buf, ios::openmode mode = ios::out);  // with initial str
*
* explicit istringstream (ios::openmode mode = ios::in);                       // default with empty string
* explicit istringstream (const string & buf, ios::openmode mode = ios::in);   // with initial string
*
* string str () const;           // Get contents
* void str (const string & str); // Set contents
*
* ----------------------------------------------------------------------------------------------------------------------------------------

****************************************************************************************************************************************
 *
 * :: -> is known as scope resolution operator. If a global variable is hidden by a local variable of same name, you could use this to retrieve the hidden global var
 * (...) -> Ellipses, can be used as the last parameter of a function to denote zero or more args of unknown type.Compiler suspends type checking for these parameters
 * *************************************************************************************************************************************/



#include <iostream>
#include <limits>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

void base_io();
void overload_cout_cin();
void fileDispaly(string filename);
void stringStream();

#define SIZEOF(val) ((char *)(&val+1) - (char *)&val)
#define DECLARE_TYPE(type) __typeof__(type) newvar

std::vector<std::string> split_str(const std::string& s, char delimeter ) {

	std::vector<std::string> v;
	int idx = 0, pos = 0;

	while ( (pos = s.find(delimeter, idx)) != std::string::npos  ) {
		if ( pos != idx ) // if token found in the same index, increment pos by 1
			v.push_back(s.substr(idx, pos-idx));
		idx = pos + 1;
	}

	if ( idx < s.length() )
		v.push_back(s.substr(idx));

	return v;
}


std::vector<std::string> split(const std::string& str, char delimeter)
{
	std::string token;
	std::istringstream iss(str);
	std::vector<std::string> v;

	while ( std::getline(iss, token, delimeter) )
		if ( token.length() > 0 )
			v.push_back(token);
	return v;
}

template<typename T>
std::vector<T> split(const std::string& str)
{
	std::istringstream iss(str);
	//std::istream_iterator<std::string> beg(iss), end;
	//std::vector<T> vec(beg, end);
	//return vec;
	return std::vector<T>( (std::istream_iterator<T>(iss)), std::istream_iterator<T>() );
}

void test_split() {

	auto vec_str = split<std::string>("this is sample string");
	std::for_each( vec_str.begin(), vec_str.end(), [](auto& val) { std::cout << val << std::endl; });

	auto vec_int(split<int>("1 2 3   4 5"));
	std::for_each( vec_int.begin(), vec_int.end(), [](auto& val) { std::cout << val << std::endl; });

	vec_str = split("1,2,3,,,4,5", ',');
	std::for_each( vec_int.begin(), vec_int.end(), [](auto& val) { std::cout << val << std::endl; });
	
	vec_str = split_str("1,2,3,,,4,5", ',');
	std::for_each( vec_int.begin(), vec_int.end(), [](auto& val) { std::cout << val << std::endl; });
}

int main()
{
	//base_io();
	//overload_cout_cin();
	//fileDispaly("delete.cpp");
	//stringStream();
	char a;
	short b;
	int c;
	double d;
	int arr[10];
	char s[50];

	cout << SIZEOF(a) << " " << SIZEOF(b) << " " << SIZEOF(c) << " " << SIZEOF(d) << " " << SIZEOF(arr) << " " << SIZEOF(s) << endl;
	cout << &arr << " " << &arr[1] << " " << &arr+1 << endl;
	cout << &s << " " << &s[1] << " " << &s+1 << endl;
	printf("\n %u %u %u", &s, &s[1], &s+1);
	test_split();
	return 0;
}

/********************************* Base I/O *****************************************************/
void base_io()
{
	int a, b, c;
	string s1, s2, s3;

	cout << "Enter an integer : ";
	cin >> a;
	cout << endl;
	cout << "The number u entered : " << a << endl;

	cout << "Enter two integers : ";
	cin >> b >> c;
	cout << endl;
	cout << "The number u entered : " << b << " , " << c << endl;

	
	cout << "Enter a string : ";
	cin >> s1;
	cout << endl;
	cout << "The string u entered : " << s1 << endl;

	cin.ignore(numeric_limits<streamsize>::max());        // Ignore to the end-of-file
	cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Ignore to the end-of-line


	cout << "Enter a string : ";
	getline(cin, s2);
	cout << endl;
	cout << "The string u entered : " << s2 << endl;
}

/********************************************************************************************************/


/************************************* cout cin overload ************************************************/
class Complex 
{ 
	private: 
		int real, imag; 
	public: 
		Complex(int r = 0, int i =0) 
		{  real = r;   imag = i; } 
		friend ostream & operator << (ostream &out, const Complex &c); 
		friend istream & operator >> (istream &in,  Complex &c); 
};

ostream & operator << (ostream &out, const Complex &c) 
{ 
	out << c.real; 
	out << "+i" << c.imag << endl; 
	return out; 
} 

istream & operator >> (istream &in,  Complex &c) 
{ 
	cout << "Enter Real Part "; 
	in >> c.real; 
	cout << "Enter Imagenory Part "; 
	in >> c.imag; 
	return in; 
}

void overload_cout_cin()
{ 
	Complex c1; 
	cin >> c1; 
	cout << "The complex object is "; 
	cout << c1; 
}

/********************************************************************************************************/

/************************************************ Streams ***********************************************/

void fileDispaly(string filename)
{
	streampos size;
	char *memblock ;

	ifstream file;
	file.open(filename.c_str(), ios::in|ios::ate);
	if ( file.is_open()) {

		size = file.tellg();
		file.seekg (0, ios::beg);
		memblock = new char[size];

		file.read(memblock, size);
		
		file.close();

		cout << memblock << endl;

		delete[] memblock;
	}else{
		cout << "Unable to open the file : " << filename << endl;
	}

	file.open(filename.c_str(), ios::in);
	if ( file.is_open()) {

		char ch;
		while ( file.get(ch) )
			cout << ch;

		file.close();

		cout << endl;

	}else{
		cout << "Unable to open the file : " << filename << endl;
	}

}

void stringStream()
{
	ostringstream sout;  

	// Write into string buffer
	sout << "apple" << endl;
	sout << "orange" << endl;
	sout << "banana" << endl;

	// Get contents
	cout << sout.str() << endl;


	// construct input string stream (buffer) - need <sstream> header
	istringstream sin("123 12.34 hello");

	// Read from buffer
	int i;
	double d;
	string s;
	sin >> i >> d >> s;
	cout << i << "," << d << "," << s << endl;
}

// C++11 varargs
template<typename T>
T adder(T v) {
  return v;
}

template <typename T, typename... Args> 
T adder(T first, Args... args) 
{ 
	return first + adder(args...) ; 
} 

// C++14 varargs
template<typename ...Args>
auto sum(Args ...args)
{
	return (... + args);
}

/********************************************************************************************************/
