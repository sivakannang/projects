#include <iostream>
#include <queue>
#include <stack>

template<typename T>
class Bst {

	public:

		struct Node {

			T data;
			Node *left;
			Node *right;

			Node(const T& data) : data(data), left(nullptr), right(nullptr) {}

			friend std::ostream& operator << ( std::ostream& os, const Node& node )
			{
				os << node.data << " ";
				return os;
			}
		};

	private:

		Node *root;

		Node *insert_(Node *node, const T &data)
		{
			if ( !node )
				return new Node(data);

			if ( data < node->data )
				node->left = insert_(node->left, data);
			else if ( data > node->data )
				node->right = insert_(node->right, data);
			else
				std::cout << "duplicate . ignored " << data << std::endl;
			return node;
		}

		Node *find_(Node *node, const T& data) const
		{
			if ( !node || data == node->data )	return node;

			if ( data < node->data )
				return find_(node->left, data);
			else
				return find_(node->right, data );
		}

		Node *remove_(Node *node, const T &data)
		{
			if ( !node )
				return nullptr;

			if ( data < node->data )
				node->left = remove_(node->left, data );
			else if ( data > node->data )
				node->right = remove_(node->right, data );
			else
			{
				if ( node->left == nullptr && node->right == nullptr )      // case 1 - no children
				{
					delete node;
					return nullptr;
				}

				if ( node->left == nullptr || node->right == nullptr )      // case 2 - one child, either left or right 
				{
					Node *tmp = node->right ? node->right : node->left ;
					delete node;
					return tmp;

				}
				
				Node *minNode = findmin_(node->right);                     // case 3 - two children, both left & right
				node->data = minNode->data;
				node->right = remove_(node->right, minNode->data);
			}
			return node;
		}


		Node *findmin_(Node *node) const
		{
			while ( node && node->left )
				node = node->left;
			return node;
		}

		Node *findmax_(Node *node) const
		{
			while ( node && node->right )
				node = node->right;
			return node;
		}

		void clear_(Node *node)
		{
			if ( !node ) return;
			clear_(node->left);
			clear_(node->right);
			std::cout << "clear " << *node;
			delete node;
		}

		void dfs_inorder_(Node *node) const
		{
			if ( !node ) return;
			dfs_inorder_(node->left);
			std::cout << *node;
			dfs_inorder_(node->right);
		}

		void dfs_preorder_(Node *node) const
		{
			if ( !node ) return;
			std::cout << *node;
			dfs_preorder_(node->left);
			dfs_preorder_(node->right);
		}

		void dfs_postorder_(Node *node) const
		{
			if ( !node ) return;
			dfs_postorder_(node->left);
			dfs_postorder_(node->right);
			std::cout << *node;
		}

		void bfs_order_(Node *node) const
		{
			if ( !node )  return;

			std::queue<Node *> q;
			q.push(node);

			while ( !q.empty() )
			{
				Node *node = q.front();
				std::cout << *node;
				q.pop();
				if ( node->left) q.push(node->left);
				if ( node->right) q.push(node->right);
			}
		}

		bool is_bst_(Node *node, T *min = nullptr, T *max = nullptr) const
		{
			if ( !node )	return true;

			if ( ( min && node->data <= *min ) || ( max && node->data >= *max ) )
				return false;

			return is_bst_(node->left, min, &node->data ) && is_bst_(node->right, &node->data, max);
		}

		bool is_leaf(Node *node) const
		{
			return node && !node->left && !node->right;
		}

		void print_leaves_(Node *node) const
		{
			if ( !node )	return;

			print_leaves_(node->left);
			if ( is_leaf(node) )
				std::cout << *node;
			print_leaves_(node->right);
		}

		void print_boundary_(Node *node) const
		{
			if ( !node )
				return ;

			// 1. Root
			std::cout << *node;

			// 2. Left boundary, excluding leaves, preorder
			print_left_boundary(node->left);

			// 3. All leaf nodes, inorder
			print_leaves_(node->left);
			print_leaves_(node->right);

			// 4. Right boundary, excluding leaves, printed bottom up
			print_right_boundary(node->right);

		}

		void print_left_boundary(Node *node) const
		{
			while ( node )
			{

				if ( !is_leaf(node) )
					std::cout << *node;

				if ( node->left )
					node = node->left;
				else
					node = node->right;
			}
		}

		void print_right_boundary(Node *node) const
		{
			std::stack<T> stk;

			while ( node )
			{
				if ( !is_leaf(node) )
					stk.push( node->data );

				if ( node->right )
					node = node->right;
				else
					node = node->left;
			}

			while ( !stk.empty() )
			{
				std::cout << stk.top() << " ";
				stk.pop();
			}
		}

	public:

		Bst() : root(nullptr) {}
		Bst(const Bst& bst) = delete;
		Bst& operator = ( const Bst& bst) = delete;
		Bst(Bst&& bst) = delete;
		Bst& operator = (Bst&& bst) = delete;
		~Bst() { clear_(root); }

		void insert(const T& data)       { root = insert_(root, data); }
		void remove(const T& data)       { root = remove_(root, data); }
		Node* find(const T& data) const  { return find_(root, data); }
		Node* findmin() const            { return findmin_(root); }
		Node* findmax() const            { return findmax_(root); }
		void clear()                     { clear_(root); root = nullptr; }
		void dfs_inorder() const         { dfs_inorder_(root); std::cout << std::endl;}
		void dfs_postorder() const       { dfs_postorder_(root); std::cout << std::endl; }
		void dfs_preorder() const        { dfs_preorder_(root); std::cout << std::endl;  }
		void bfs_order() const           { bfs_order_(root); std::cout << std::endl; }
		bool is_bst() const              { return is_bst_(root); }
		void print_leaves() const        { print_leaves_(root); std::cout << std::endl; }
		void print_boundary() const      { print_boundary_(root); std::cout << std::endl; }

};


int main()
{
	Bst<int> bst;

	bst.insert(5);
	bst.insert(3);
	bst.insert(8);
	bst.insert(2);
	bst.insert(4);
	bst.insert(7);
	bst.insert(9);
	bst.insert(7);



	bst.dfs_inorder();
	bst.dfs_preorder();
	bst.dfs_postorder();
	bst.remove(3);
	bst.dfs_postorder();
	bst.bfs_order();
	auto node = bst.find(2);
	if ( node )
		std::cout << "bst.find(2) = " << *node << std::endl;

	std::cout << "is_bst " << bst.is_bst() << std::endl;

	return 0;
}
