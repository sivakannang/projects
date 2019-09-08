
#include <iostream>
#include <vector>
#include <exception>
#include <cstdlib>

#include "generic.h"

int stack_size = 10;

/****************** STACK ********************
 * Will see two variations
 *  i.  Fixed size with dynamic type using templates
 *  ii. Dynamic size with dynamic type templates and pointers
 ********************************************/


template<typename T>
bool STACK<T>::push(T t) {

	try {
		if ( isFull() )
			throw std::overflow_error("overflow error");
		v.push_back(t);
		traverse();
	}catch(std::overflow_error &e){
		std::cout << e.what() << std::endl;
		return false;
	}
	return true;
}

template<typename T>
bool STACK<T>::pop() {

	try {
		if ( isEmpty() )
			throw std::underflow_error("underflow error");
		v.pop_back();
		traverse();
		return true;
	}
	catch(std::underflow_error &e) {
		std::cout << e.what() << std::endl;
	}catch(std::exception &e){
		std::cout << "exception : " << e.what() << std::endl;
	}
	return false;
}

template<typename T>
T STACK<T>::top() const {

	try {
		if ( isEmpty() )
			throw std::underflow_error("underflow error");
		std::cout << v.at(v.size()-1) << std::endl;
		return v.at(v.size()-1);
	}
	catch(std::underflow_error &e) {
		std::cout << e.what() << std::endl;
	}catch(std::exception &e){
		std::cout << "exception : " << e.what() << std::endl;
	}
}

template<typename T>
void STACK<T>::traverse() const {

	std::cout << std::endl;

	for ( const T &t : v )
		std::cout << t << " ";

	std::cout << std::endl;

}


template<typename T>
bool QUEUE<T>::push(T t) {

	try {
		if ( isFull() )
			throw std::overflow_error("overflow error");

		Node<T> *newNode = new Node<T>(t, nullptr);
		if ( frontNode == nullptr ) {
			rearNode = frontNode = newNode;
		} else {
			rearNode->right = newNode;
			rearNode = newNode;
		}
		cur_size++;
		traverse();
		
	}catch(std::overflow_error &e){
		std::cout << e.what() << std::endl;
		return false;
	}
	return true;
}

template<typename T>
bool QUEUE<T>::pop() {

	try {
		if ( isEmpty() )
			throw std::underflow_error("underflow error");
		Node<T> *tmp = frontNode;
		frontNode = frontNode->right;
		delete tmp;
		cur_size--;
		traverse();
		return true;
	}
	catch(std::underflow_error &e) {
		std::cout << e.what() << std::endl;
	}catch(std::exception &e){
		std::cout << "exception : " << e.what() << std::endl;
	}
	return false;
}

template<typename T>
T QUEUE<T>::top() const {

	try {
		if ( isEmpty() )
			throw std::underflow_error("underflow error");
		std::cout << frontNode->data << std::endl;
		return frontNode->data;
	}
	catch(std::underflow_error &e) {
		std::cout << e.what() << std::endl;
	}catch(std::exception &e){
		std::cout << "exception : " << e.what() << std::endl;
	}
}

template<typename T>
void QUEUE<T>::traverse() const {

	std::cout << std::endl;

	Node<T> *head = getHead();
	while ( head ) {
		std::cout << head->data << " " << std::endl;
		head = head->right;
	}

	std::cout << std::endl;

}

int main() {

	int ops = 0;
	//STACK<char> s(5);
	//QUEUE<char> s;
	QUEUE<Employee> s;
	Employee e;

	/*do {
		std::cout << std::endl << "STACK OPERATIONS" << std::endl;
		std::cout << "1. Push" << std::endl;
		std::cout << "2. Pop"  << std::endl;
		std::cout << "3. Top"  << std::endl;
		std::cout << "4. Traverse" << std::endl;
		std::cout << "5. Exit" << std::endl;
		std::cout << "Enter choice(1 to 5) : ";

		std::cin >> ops;

		switch(ops) {
			//case 1: s.push( (rand() % 25) + 'a'); break;
			case 1: std::cin >> e; s.push(e); break;
			case 2: s.pop(); break;
			case 3: s.top(); break;
			case 4: s.traverse(); break;
			case 5: return 0;
			default: std::cout << "Invalid choice - Please retry" << std::endl; break;
		}
	}while ( ops != 5);*/

	/*BST<Employee> bst;
	std::cin >> e;
	bst.insert(e);
	std::cin >> e;
	bst.insert(e);
	std::cin >> e;
	bst.insert(e);
	std::cin >> e;
	bst.insert(e);*/
	test_array();

}


template <typename T>
Node<T> *Tree<T>::_insert(Node<T> *node, T &data) {
	
	if ( node == nullptr ) 
		node = new Node<T>(data, nullptr, nullptr);
	else if ( data < node->data )
		node->left = _insert(node->left, data);
	else if ( data > node->data )
		node->right = _insert(node->right, data);
	else
		std::cout << "duplicate key not allowed " << std::endl;

	return node;
}

template <typename T>
void Tree<T>::_dfs_in_order(Node<T> *node) {
	if ( node == nullptr )
		return;

	_dfs_in_order(node->left);
	std::cout << node->data;
	_dfs_in_order(node->right);
}



void test_array() {
	
	ARRAY<int, 10> arr;

	for ( int i = 0; i<arr.size(); i++)
		std::cout << arr[i] << " ";

	for ( int i = 0; i<arr.size(); i++)
		arr[i] = rand() % 255;

	for ( int i = 0; i<arr.size(); i++)
		std::cout << arr[i] << " ";
	std::cout << std::endl;
	
}
