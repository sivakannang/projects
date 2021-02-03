

#include <iostream>
#include <string>
#include <stack>


class Compiler {

	public:
		bool isBalanced(std::string str);
};

bool Compiler::isBalanced(std::string str){

	std::stack<char> s;

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

// Tracking current minimum element in stack
template<typename T>
class stack_min {

	std::stack<T> stack;
	std::stack<T> cache;

	public:

	T min() const { return cache.top(); }

	void push(T data) {
		stack.push(data);

		if ( stack.size() == 1) {
			cache.push(data);
			return;
		}

		if ( data <= cache.top() )
			cache.push(data);
	}

	void pop() {
		T data = stack.top();
		stack.pop();

		if ( data == cache.top())
			cache.pop();
	}

};

int main(){

	std::string str;
	Compiler compiler;

	while ( std::getline(std::cin, str) ) {
		std::cout << compiler.isBalanced(str) << std::endl;
	}

	return 0;
}

