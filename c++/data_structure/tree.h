#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <vector>

typedef struct NODE {
	int data;
	struct NODE *left;
	struct NODE *right;
} Node;


class Tree {

	protected:
		Node *root;

		void in_order(Node *node);
		void pre_order(Node *node);
		void post_order(Node *node);
		Node *search(Node *node, int data);
		void clear(Node *node);
		Node *find_min(Node *node);
		Node *find_max(Node *node);
		unsigned long height(Node *node);
		unsigned long leafCount(Node *node);
		Node *rr_rotation(Node *parent);
		Node *ll_rotation(Node *parent);
		Node *lr_rotation(Node *parent);
		Node *rl_rotation(Node *parent);
		Node *balance(Node *node);

	public:
		Tree() { root = nullptr; }
		~Tree() { clear(root); }

		// A default copy would share the nodes, and both trees would delete them
		Tree(const Tree&) = delete;
		Tree& operator=(const Tree&) = delete;

		void display() { in_order(root); std::cout << std::endl; }
		void level_order();
		Node *search(int data) { return search(root, data); }
		unsigned long height() { return height(root); }
		unsigned long leafCount() { return leafCount(root); }
		long diff(Node *node);
		Node *Insert(Node *node, int data, bool balanceRequired = false);
		bool is_bst(Node *node);
};

class BST : public Tree {
	private:
		Node *remove(Node *root, int data);
	public:
		BST() : Tree() {}
		void insert(int data) { root = Insert(root, data); }
		void remove(int data) { root = remove(root, data); }   // The root itself can change
		bool isBST() { return is_bst(root); }
};

class AVL : public Tree {
	public:
		AVL() : Tree() {}
		void insert(int data) { root = Insert(root, data, true); }
		bool isBST() { return is_bst(root); }
};


class PriorityQueue {
	private:

		std::vector<int> v;                          // vector to store heap elements

		int parent(int idx) { return (idx-1) / 2; }   // don't call this function if idx is root node
		int left  (int idx) { return (idx*2) + 1; }
		int right (int idx) { return (idx*2) + 2; }
		void heapify_up(int idx);
		void heapify_down(unsigned int idx);

	public:
		unsigned int size() { return v.size();    }
		bool empty()        { return size() == 0; }
		int top();                                    // Throws std::out_of_range if empty
		void push(int key);
		void pop();                                   // Throws std::out_of_range if empty
};

#endif
