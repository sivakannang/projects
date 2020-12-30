

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
		Tree() { root = NULL; };
		~Tree() { std::cout << "~Tree invoked" << std::endl; clear(root); }
		void display() { in_order(root); std::cout << std::endl; /*pre_order(root);  std::cout << std::endl;  post_order(root); std::cout << std::endl;*/ }
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
		void remove(int data) { remove(root, data); }
		bool isBST() { return is_bst(root); }
};

class AVL : public Tree {
	private:
		//Node *remove(Node *root, int data);

	public:
		AVL() : Tree() {}
		void insert(int data) { root = Insert(root, data, true); }
		//void remove(int data) { remove(root, data); }
};


class PriorityQueue {
	private:
		
		std::vector<int> v;                          // vector to store heap elements

		int parent(int idx) { return (idx-1) / 2; }   // don't call this function if idx is root node
		int left  (int idx) { return (idx*2) + 1; }
		int right (int idx) { return (idx*2) + 2; }
		void heapify_up(int idx);
		void heapify_down(int idx);

	public:
		unsigned int size() { return v.size();    }
		bool empty()        { return size() == 0; }
		int top();
		void push(int key);
		void pop();
};



