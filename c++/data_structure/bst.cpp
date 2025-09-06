#include <iostream>
#include <queue>
#include <stack>
#include <algorithm> // std::max
#include <cstddef>   // std::size_t
#include <cstdlib>   // std::abs

template<typename T>
class Bst {
	public:
		struct Node {
			T data;
			Node *left;
			Node *right;
			Node(const T& data) : data(data), left(nullptr), right(nullptr) {}
			friend std::ostream& operator<<(std::ostream& os, const Node& n) { os << n.data << " "; return os; }
		};

	private:
		Node *root{nullptr};

		Node* insert(Node* n, const T& x) {
			if (!n) return new Node(x);
			if (x < n->data)       n->left  = insert(n->left,  x);
			else if ( x > n->data )  n->right = insert(n->right, x);
			else std::cout << "duplicate . ignored " << x << std::endl;
			return n;
		}
		Node* find(Node* n, const T& x) const {
			if (!n || x == n->data) return n;
			return (x < n->data) ? find(n->left, x) : find(n->right, x);
		}
		static Node* findmin(Node* n) {
			while (n && n->left) n = n->left;
			return n;
		}
		static Node* findmax(Node* n) {
			while (n && n->right) n = n->right;
			return n;
		}
		Node* remove(Node* n, const T& x) {
			if (!n) return nullptr;
			if (x < n->data)         n->left  = remove(n->left,  x);
			else if ( x > n->data )  n->right = remove(n->right, x);
			else {
				if (!n->left && !n->right) { delete n; return nullptr; }     // case 1 - no child
				if (!n->left || !n->right) {                                 // case 2 - one child
					Node* c = n->left ? n->left : n->right;
					delete n; return c;
				}
				Node* s = findmin(n->right);                                 // case 3 - two child
				n->data = s->data;
				n->right = remove(n->right, s->data);
			}
			return n;
		}
		static void clear(Node* n) {
			if (!n) return;
			clear(n->left);
			clear(n->right);
			delete n;
		}
		void dfs_inorder(Node* n) const {
			if (!n) return;
			dfs_inorder(n->left);
			std::cout << *n;
			dfs_inorder(n->right);
		}
		void dfs_preorder(Node* n) const {
			if (!n) return;
			std::cout << *n;
			dfs_preorder(n->left);
			dfs_preorder(n->right);
		}
		void dfs_postorder(Node* n) const {
			if (!n) return;
			dfs_postorder(n->left);
			dfs_postorder(n->right);
			std::cout << *n;
		}
		void bfs_order(Node* n) const {
			if (!n) return;
			std::queue<Node*> q; q.push(n);
			while (!q.empty()) {
				Node* cur = q.front(); q.pop();
				std::cout << *cur;
				if (cur->left)  q.push(cur->left);
				if (cur->right) q.push(cur->right);
			}
		}
		bool is_bst(Node* n, const T* mn=nullptr, const T* mx=nullptr) const {
			if (!n) return true;
			if ((mn && n->data <= *mn) || (mx && n->data >= *mx)) return false;
			return is_bst(n->left, mn, &n->data) && is_bst(n->right, &n->data, mx);
		}
		static bool is_leaf(Node* n) { return n && !n->left && !n->right; }
		void print_leaves(Node* n) const {
			if (!n) return;
			print_leaves(n->left);
			if (is_leaf(n)) std::cout << *n;
			print_leaves(n->right);
		}
		void print_left_boundary(Node* n) const {
			while (n) {
				if (!is_leaf(n)) std::cout << *n;
				n = n->left ? n->left : n->right;
			}
		}
		void print_right_boundary(Node* n) const {
			std::stack<const Node*> st;
			while (n) {
				if (!is_leaf(n)) st.push(n);
				n = n->right ? n->right : n->left;
			}
			while (!st.empty()) { std::cout << *st.top(); st.pop(); }
		}
		void print_boundary(Node* n) const {
			if (!n) return;
			std::cout << *n;                  // root
			print_left_boundary(n->left);     // left boundary
			print_leaves(n->left);            // leaves
			print_leaves(n->right);
			print_right_boundary(n->right);   // right boundary
		}

		// -------- Added utilities --------
		int height(Node* n) const {
			return n ? 1 + std::max(height(n->left), height(n->right)) : 0;
		}
		bool is_balanced(Node* n) const {
			if (!n) return true;
			int hl = height(n->left), hr = height(n->right);
			if (std::abs(hl - hr) > 1) return false;
			return is_balanced(n->left) && is_balanced(n->right);
		}
		Node* lower_bound(Node* n, const T& x) const {
			Node* ans = nullptr;
			while (n) {
				if ( n->data >= x) { ans = n; n = n->left; }
				else n = n->right;
			}
			return ans;
		}
		Node* upper_bound(Node* n, const T& x) const {
			Node* ans = nullptr;
			while (n) {
				if ( n->data > x ) { ans = n; n = n->left; }
				else n = n->right;
			}
			return ans;
		}
		Node* successor(Node *n, const T& x) const { // smallest > x
			Node* ans = nullptr;
			while (n) {
				if ( n->data > x ) { ans = n; n = n->left; }
				else n = n->right;
			}
			return ans;
		}
		Node* predecessor(Node *n, const T& x) const { // largest < x
			Node* ans = nullptr;
			while (n) {
				if ( n->data < x ) { ans = n; n = n->right; }
				else n = n->left;
			}
			return ans;
		}
		Node* lca(Node* n, const T& a, const T& b) const {
			const T& lo = a > b ? b : a;
			const T& hi = a > b ? a : b;

			while (n) {
				if ( lo > n->data) n = n->right;
				else if ( hi < n->data ) n = n->left;
				else return n; // a <= n->data <= b
			}
			return nullptr;
		}
		Node* kth_smallest(Node* n, std::size_t& k) const {
			if (!n) return nullptr;
			if (Node* L = kth_smallest(n->left, k)) return L;
			if (--k == 0) return n;
			return kth_smallest(n->right, k);
		}

		static const Node* kth_smallest(const Node* n, std::size_t k) {
			if (!n || k == 0) return nullptr;

			std::stack<const Node*> st;
			const Node* cur = n;

			while (cur || !st.empty()) {
				// go all the way left
				while (cur) {
					st.push(cur);
					cur = cur->left_;
				}
				// visit node
				cur = st.top(); st.pop();
				if (--k == 0) return cur;   // k-th smallest

				// then go right
				cur = cur->right_;
			}
			return nullptr; // k > number of nodes
		}
		void print_range(Node* n, const T& L, const T& R) const {
			if (!n) return;
			if (L < n->data) print_range(n->left, L, R);
			if (!(n->data < L) && !(R < n->data)) std::cout << n->data << " ";
			if (n->data < R) print_range(n->right, L, R);
		}

	public:
		Bst() = default;
		Bst(const Bst&) = delete;
		Bst& operator=(const Bst&) = delete;
		Bst(Bst&&) = delete;
		Bst& operator=(Bst&&) = delete;
		~Bst() { clear(root); }

		// core ops
		void  insert(const T& x)      { root = insert(root, x); }
		void  remove(const T& x)      { root = remove(root, x); }
		Node* find (const T& x) const { return find(root, x); }
		Node* findmin() const         { return findmin(root); }
		Node* findmax() const         { return findmax(root); }
		void  clear()                 { clear(root); root = nullptr; }

		// traversals / checks
		void dfs_inorder()  const { dfs_inorder(root);  std::cout << '\n'; }
		void dfs_preorder() const { dfs_preorder(root); std::cout << '\n'; }
		void dfs_postorder() const{ dfs_postorder(root);std::cout << '\n'; }
		void bfs_order()    const { bfs_order(root);    std::cout << '\n'; }
		bool is_bst()       const { return is_bst(root); }
		void print_leaves() const { print_leaves(root); std::cout << '\n'; }
		void print_boundary()const{ print_boundary(root); std::cout << '\n'; }

		// -------- Public wrappers for new utilities --------
		int   height()                  const { return height(root); }
		bool  is_balanced()             const { return is_balanced(root); }
		Node* lower_bound(const T& x)   const { return lower_bound(root, x); }
		Node* upper_bound(const T& x)   const { return upper_bound(root, x); }
		Node* successor(const T& x)     const { return successor(root, x); }
		Node* predecessor(const T& x)   const { return predecessor(root, x); }
		Node* lca(const T& a, const T& b) const { return lca(root, a, b); }
		Node* kth_smallest(std::size_t k) const { return kth_smallest(root, k); }
		void  print_range(const T& L, const T& R) const { print_range(root, L, R); std::cout << '\n'; }
		Node* get() const { return root; }
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
