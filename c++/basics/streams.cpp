
/******************************* C++ Streams **************************************************************
*
*  - std::cin, std::cout, std::cerr, std::clog: console I/O  ( cerr is unbuffered )
*
*  - File streams:
*    * std::ifstream - for file input
*    * std::ofstream - for file output
*    * std::fstream  - for file input + output
*
*  - String streams:
*    * std::istringstream - input from string
*    * std::ostringstream - output to string
*    * std::stringstream  - both input/output to string
*
*  - Stream manipulators:
*    * std::setw(n), std::setfill(c), std::setprecision(n)
*    * std::fixed, std::hex, std::dec, std::oct, std::showpoint
*
*
* ios::in    - open file for input operation
* ios::out   - open file for output operation
* ios::app   - output appends at the end of the file.
* ios::trunc - truncate the file and discard old contents.
* ios::binary- for binary (raw byte) IO operation, instead of character-based.
* ios::ate   - position the file pointer "at the end" for input/output.
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
* std::ios::beg (offset from the beginning of the stream's buffer).
* std::ios::cur (offset from the current position in the stream's buffer).
* std::ios::end (offset from the end of the stream's buffer).
*
*
* - :: → Scope resolution operator, used to access a hidden global variable.
* - (...) → Ellipsis, allows functions to accept variable number of arguments (type checking disabled).
*
************************************** Interview Questions and Answers ****************************
*
*  1. What is the difference between std::cin and std::ifstream?
*     - std::cin is for console input; ifstream is for reading from files.
*
*  2. What are the uses of std::fstream?
*     - It can read and write to the same file stream.
*
*  3. How to format numbers using manipulators?
*     - Use fixed, setprecision, setw, setfill, etc.
*
*  4. What is a stringstream?
*     - A stream that operates on strings instead of files or console.
*
*  5. When do you use istringstream and ostringstream?
*     - istringstream is used to extract data from a string.
*     - ostringstream is used to format data into a string.
*
*  6. How to check if a file opened successfully?
*     - Use `.is_open()` or check stream validity with if/else.
*
*  7. What does flush do?
*     - It forces buffer to be written immediately.
*
*  8. How do you open a file in binary mode?
*     - Use `std::ios::binary` when opening the file.
*
*  9. What does std::getline do?
*     - It reads a full line of text (including spaces).
*
*  10. How to write binary data to file?
*     - Use `ostream.write(char*, size)` and `istream.read(char*, size)`.
*
*
*-------------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

void fileStreamDemo() {
	// Write to file
	std::ofstream fout("example.txt");
	fout << "Value: " << std::setw(5) << std::setfill('0') << 42 << "\n";
	fout.close();

	std::ifstream fin("example.txt");
	std::string line;
	while (std::getline(fin, line)) {
		std::cout << "From file: " << line << std::endl;
	}
	fin.close();

	// Read file char-by-char
	std::ifstream charin("example.txt");
	char ch;
	std::cout << "Char-by-char: ";
	while (charin.get(ch)) {
		std::cout << ch;
	}
	charin.close();

	// Binary write and read
	const char* binData = "ABCD"; // ABCD in ASCII
	std::ofstream bout("binary.dat", std::ios::binary);
	bout.write(binData, 4);
	bout.close();

	std::ifstream binin("binary.dat", std::ios::binary);
	char buffer[4];
	binin.read(buffer, 4);
	std::cout << "Binary read: ";
	for (int i = 0; i < 4; ++i) {
		std::cout << buffer[i] << ' ';
	}
	std::cout << std::endl;
	binin.close();
}

void stringStreamDemo() {
	std::string input = "123 45.6 Hello";
	std::istringstream iss(input);
	int i; double d; std::string s;
	iss >> i >> d >> s;
	std::cout << "Parsed: " << i << ", " << d << ", " << s << std::endl;

	std::ostringstream oss;
	oss << "Formatted output: [" << std::setw(8) << std::left << s << "]";
	std::string result = oss.str();
	std::cout << result << std::endl;
}

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
int main() {
	fileStreamDemo();
	stringStreamDemo();
	return 0;
}
