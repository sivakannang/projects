
/******************************************************************** Tree ***************************************************************
 *
 *
 * 			insert		delete		search
 * BST                  Olog(n)
 *
 *
 *
 */
/*
 * binary search tree:
 * https://gist.github.com/mycodeschool/44e1a9183ab0e931f729
 * https://www.youtube.com/watch?v=COZK7NATh4k&t=19s
 *                          5
			   / \
			  3   10
			 / \   \
			1   4   11
 *
 *
 * AVL Tree
 * https://www.sanfoundry.com/cpp-program-implement-avl-trees/
 *
 * A Tree is typically traversed in two ways:
 *
 * Breadth First Traversal (Or Level Order Traversal)
 * Depth First Traversals
 * 	Inorder Traversal (Left-Root-Right)
 * 	Preorder Traversal (Root-Left-Right)
 * 	Postorder Traversal (Left-Right-Root)
 *
 *      Binary Heap ( Priority queue )
 *      https://www.fluentcpp.com/2018/03/13/heaps-priority-queues-stl-part-1/
 *      https://www.cs.usfca.edu/~galles/visualization/Heap.html
 * 	https://www.tutorialspoint.com/data_structures_algorithms/heap_data_structure.htm
 */




/*
1.) left <= root < right

2.) left < root <= right

3.) left < root < right, such that no duplicate keys exist
*/

#include <iostream>
#include <vector>
#include <exception>
#include "tree.h"

void heap_binary_max_impl_test();

using namespace std;

Node *Tree::Insert(Node *node, int data, bool balanceRequired) {

	if ( node == NULL ) {
		node = new Node;
		node->data = data;
		node->left = node->right = NULL;
	} else if ( data < node->data ) {
		node->left = Insert(node->left, data, balanceRequired);
		if ( balanceRequired )  node = balance(node);
	} else if ( data > node->data ) {
		node->right = Insert(node->right, data, balanceRequired);
		if ( balanceRequired )  node = balance(node);
	} else {
		std::cout << "dup key/data not allowed" << std::endl;
	}

	return node;
}

void Tree::in_order(Node *node) {

	if ( node == NULL)
		return;

	in_order(node->left);
	std::cout << node->data << " ";
	in_order(node->right);
}

void Tree::pre_order(Node *node) {

	if ( node == NULL)
		return;

	std::cout << node->data << " ";
	pre_order(node->left);
	pre_order(node->right);
}


void Tree::post_order(Node *node) {

	if ( node == NULL)
		return;

	post_order(node->left);
	post_order(node->right);
	std::cout << node->data << " ";
}

void level_order(Node *node) {
    
	if ( node == nullptr )
		return;
	
	std::queue<Node *> q;
	q.push(node);

    while ( !q.empty() ) {
		
		Node *node = q.front();
		std::cout << node->data << std::endl;
		q.pop();
		
		if ( node->left )
			q.push(node->left)
		if ( node->right )
			q.push(node->right);
	}		
}

unsigned long Tree::height(Node *node) {

	if ( node == NULL)  return 0;

	unsigned long lh = height(node->left);
	unsigned long rh = height(node->right);

	return 1 + std::max(lh, rh);
		
}

long Tree::diff(Node *node) {

	// b_factor = height(left) - height(right)
	return height(node->left) - height(node->right);
}

unsigned long Tree::leafCount(Node *node) {

	if ( node == NULL ) return 0;
	
	if ( node->left == NULL && node->right == NULL )
		return 1;
	else
		return leafCount(node->left) + leafCount(node->right);
}




void Tree::clear(Node *node) {

	if ( node == NULL)	return ;

	clear(node->left);
	clear(node->right);

	delete node;
}

Node *Tree::search(Node *node, int data) {
	
	if ( node == NULL) {

		std::cout << "search(" << data << ") = not exists " << std::endl;
		return NULL;
	}

	if ( data < node->data)
		return search(node->left, data);
	else if ( data > node->data)
		return search(node->right, data);
	else {
		std::cout << "search(" << data << ") = exists " << std::endl;
		return node;
	}
}


Node *BST::remove(Node *node, int data) {

	if ( node == NULL ) return NULL;

	if ( data < node->data)
		node->left = remove(node->left, data);
	else if ( data > node->data)
		node->right = remove(node->right, data);
	else { // we found it

		Node *tmp = node;

		if ( node->left == NULL && node->right == NULL ) {		    // case 1 - no child
			delete node;
			node = NULL;
		} else if ( node->left == NULL || node->right == NULL ) {           // case 2 - one child
			node =  node->left ? node->left : node->right;
			delete tmp;
		}
		else {                                                              // case 2 - two child's
			tmp = find_min(node->right);                 // 1. find the min value node
			node->data = tmp->data;                      // 2. copy the min value node to current node
			node->right = remove(tmp->right, tmp->data); // 3. remove the duplicated min value node from current node->right
			                                             // 4. If you want to arrange it towards node->left side, then we should find max value
		}
	}

	return node;

}

Node *Tree::find_min(Node *node) {

	while ( node->left) node = node->left;

	return node;
}

Node *Tree::find_max(Node *node) {

	while ( node->right ) node = node->right;

	return node;
}

Node *Tree::rr_rotation(Node *parent)
{
    Node *tmp = parent->right;
    parent->right = tmp->left;
    tmp->left = parent;
    return tmp;
}

Node *Tree::ll_rotation(Node *parent)
{
    Node *tmp = parent->left;
    parent->left = tmp->right;
    tmp->right = parent;
    return tmp;
}

Node *Tree::lr_rotation(Node *parent)
{
    Node *tmp = parent->left;
    parent->left = rr_rotation (tmp);
    return ll_rotation (parent);
}
 
Node *Tree::rl_rotation(Node *parent)
{
    Node *tmp = parent->right;
    parent->right = ll_rotation (tmp);
    return rr_rotation (parent);
}

Node *Tree::balance(Node *node)
{
    int bal_factor = diff (node);
    if (bal_factor > 1)
    {
        if (diff (node->left) > 0)
            node = ll_rotation (node);
        else
            node = lr_rotation (node);
    }
    else if (bal_factor < -1)
    {
        if (diff (node->right) > 0)
            node = rl_rotation (node);
        else
            node = rr_rotation (node);
    }
    return node;
}

bool Tree::is_bst(Node *node) {
	
	static Node *prev = NULL;

	if ( node == NULL )
		return true;

	if ( !is_bst( node->left) )
		return false;

	if ( prev != NULL && node->data <= prev->data )
		return false;

	prev = node;

	return is_bst( node->right);
}


void PriorityQueue::heapify_up(int idx) {

	if ( idx && (v[idx] > v[parent(idx)]) ){             // max heap binary property violation. child value is greater than parent
		std::swap(v[idx], v[parent(idx)]);           // swap and call recursively
		heapify_up(parent(idx));
	}
}

void PriorityQueue::heapify_down(int idx) {
	
	unsigned int largest = idx;
	unsigned int left_idx = left(idx);                            // get right and left node index
	unsigned int right_idx = right(idx);

	if ( left_idx < size() && v[left_idx] > v[idx] )     // check if left node value is greater than parent node
		largest = left_idx;

	if ( right_idx < size() && v[right_idx] > v[idx] )   // check if right node value is greater than parent node
		largest = right_idx;

	if ( idx != largest) {                               // Swap, if child node value is greater than parent node, and call recursively
		std::swap(v[idx], v[largest]);
		heapify_down(largest);
	}


}

int PriorityQueue::top() {

	try {
		if ( size() == 0)
			throw out_of_range("Vector<X>::at() : Index is out of range(Heap Underflow)");
		return v.at(0);
	}catch(const out_of_range& ex) {
		std::cout << ex.what() << std::endl;
	}
}

void PriorityQueue::push(int key) {

	v.push_back(key);
	int idx = size() - 1;
	heapify_up(idx);
}

void PriorityQueue::pop() {
	try {
		if ( size() == 0)
			throw out_of_range("Vector<X>::pop() : Index is out of range(Heap Underflow)");
		v[0] = v.back(); // Replace the root of the heap with last element of the vector
		v.pop_back();
		heapify_down(0); // call heapify_down on root node
	} catch ( const out_of_range& ex) {
		std::cout << ex.what() << std::endl;
	}
}


void heap_binary_max_impl_test() {

	PriorityQueue pq;
	
	// Note - Priority is decided by element's value
	
	pq.push(3);
	pq.push(2);
	pq.push(15);
	
	cout << "Size is " << pq.size() << endl;	
	
	cout << pq.top() << " ";
	pq.pop();

	cout << pq.top() << " ";
	pq.pop();

	pq.push(5);
	pq.push(4);
	pq.push(45);

	cout << endl << "Size is " << pq.size() << endl;

	//for(auto it = pq.begin(); it != pq.end(); it++)
		//cout << *it;
	cout << endl;
	
	cout << pq.top() << " ";
	pq.pop();

	cout << pq.top() << " ";
	pq.pop();

	cout << pq.top() << " ";
	pq.pop();

	cout << pq.top() << " ";
	pq.pop();

	cout << endl << std::boolalpha << pq.empty();
	
	pq.top();	// top operation on an empty heap
	pq.pop();	// pop operation on an empty heap

}



int main() {

	BST bst;

	bst.insert(5);
	bst.insert(10);
	bst.insert(3);
	bst.insert(4);
	bst.insert(1);
	bst.insert(11);
	std::cout << bst.leafCount() << " " << bst.height() << std::endl;

	bst.display();

	for ( int i = 0; i <= 15; i++)
		bst.search(i);
	
	//case 1 - no child
	bst.display();
	bst.remove(11);
	bst.display();

	//case 2 - one child
	bst.insert(11);
	bst.display();
	bst.remove(10);

	//case 3 : 2 childs
	bst.display();
	bst.insert(10);
	bst.display();
	bst.remove(3);
	bst.display();
	bst.insert(3);
	bst.display();
	std::cout << bst.height() << " " << bst.leafCount() << std::endl;

	std::cout << "isBST : " << std::boolalpha << bst.isBST() << std::endl;

	AVL avl;
	for ( int i = 10; i < 100; i++ )
		avl.insert(i);
	avl.display();
	std::cout << avl.height() << " " << avl.leafCount() << std::endl;

	heap_binary_max_impl_test();


	return 0;
}
