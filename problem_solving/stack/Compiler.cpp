

#include <iostream>
#include <string>
#include <stack>

using namespace std;


class Compiler {

	public:
		bool isBalanced(string str);
};

bool Compiler::isBalanced(string str){

	stack<char> s;

	for ( auto c: str) {
	
		switch(c) {

			case '{':
			case '(':
			case '[':
				s.push(c);
				break;

			case '}':
				if ( s.empty() || s.top() != '{' )
					return false;
				s.pop();
				break;

			case ']':
				if ( s.empty() || s.top() != '[' )
					return false;
				s.pop();
				break;

			case ')':
				if ( s.empty() || s.top() != '(' )
					return false;
				s.pop();
				break;
		}
	}

	return s.empty() ? true : false;
}

int main(){

	string str;
	Compiler compiler;

	while ( getline(cin, str) ) {
		cout << compiler.isBalanced(str) << endl;
	}

	return 0;
}

