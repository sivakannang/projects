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
			friend std::ostream& operator<<(std::ostream& os, const Node& n) {
				os << n.data << " ";
				return os;
			}
		};

	private:
		Node *root{nullptr};

		Node* insert_(Node* n, const T& x) {
			if (!n) return new Node(x);
			if (x < n->data)       n->left  = insert_(n->left,  x);
			else if (n->data < x)  n->right = insert_(n->right, x);
			else std::cout << "duplicate . ignored " << x << std::endl;
			return n;
		}
		Node* find_(Node* n, const T& x) const {
			if (!n || x == n->data) return n;
			return (x < n->data) ? find_(n->left, x) : find_(n->right, x);
		}
		static Node* findmin_(Node* n) {
			while (n && n->left) n = n->left;
			return n;
		}
		static Node* findmax_(Node* n) {
			while (n && n->right) n = n->right;
			return n;
		}
		Node* remove_(Node* n, const T& x) {
			if (!n) return nullptr;
			if (x < n->data)       n->left  = remove_(n->left,  x);
			else if (n->data < x)  n->right = remove_(n->right, x);
			else {
				if (!n->left && !n->right) { delete n; return nullptr; }
				if (!n->left || !n->right) {
					Node* c = n->left ? n->left : n->right;
					delete n; return c;
				}
				Node* s = findmin_(n->right);
				n->data = s->data;
				n->right = remove_(n->right, s->data);
			}
			return n;
		}
		static void clear_(Node* n) {
			if (!n) return;
			clear_(n->left);
			clear_(n->right);
			delete n;
		}
		void dfs_inorder_(Node* n) const {
			if (!n) return;
			dfs_inorder_(n->left);
			std::cout << *n;
			dfs_inorder_(n->right);
		}
		void dfs_preorder_(Node* n) const {
			if (!n) return;
			std::cout << *n;
			dfs_preorder_(n->left);
			dfs_preorder_(n->right);
		}
		void dfs_postorder_(Node* n) const {
			if (!n) return;
			dfs_postorder_(n->left);
			dfs_postorder_(n->right);
			std::cout << *n;
		}
		void bfs_order_(Node* n) const {
			if (!n) return;
			std::queue<Node*> q; q.push(n);
			while (!q.empty()) {
				Node* cur = q.front(); q.pop();
				std::cout << *cur;
				if (cur->left)  q.push(cur->left);
				if (cur->right) q.push(cur->right);
			}
		}
		bool is_bst_(Node* n, const T* mn=nullptr, const T* mx=nullptr) const {
			if (!n) return true;
			if ((mn && n->data <= *mn) || (mx && n->data >= *mx)) return false;
			return is_bst_(n->left, mn, &n->data) && is_bst_(n->right, &n->data, mx);
		}
		static bool is_leaf(Node* n) { return n && !n->left && !n->right; }
		void print_leaves_(Node* n) const {
			if (!n) return;
			print_leaves_(n->left);
			if (is_leaf(n)) std::cout << *n;
			print_leaves_(n->right);
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
		void print_boundary_(Node* n) const {
			if (!n) return;
			std::cout << *n;                  // root
			print_left_boundary(n->left);     // left boundary
			print_leaves_(n->left);           // leaves
			print_leaves_(n->right);
			print_right_boundary(n->right);   // right boundary
		}

		// -------- Added utilities --------
		int height_(Node* n) const {
			return n ? 1 + std::max(height_(n->left), height_(n->right)) : 0;
		}
		bool is_balanced_(Node* n) const {
			if (!n) return true;
			int hl = height_(n->left), hr = height_(n->right);
			if (std::abs(hl - hr) > 1) return false;
			return is_balanced_(n->left) && is_balanced_(n->right);
		}
		Node* lower_bound_(Node* n, const T& x) const {
			Node* ans = nullptr;
			while (n) {
				if (!(n->data < x)) { ans = n; n = n->left; }
				else n = n->right;
			}
			return ans;
		}
		Node* upper_bound_(Node* n, const T& x) const {
			Node* ans = nullptr;
			while (n) {
				if (x < n->data) { ans = n; n = n->left; }
				else n = n->right;
			}
			return ans;
		}
		Node* successor_(const T& x) const { // smallest > x
			Node* cur = root; Node* succ = nullptr;
			while (cur) {
				if (x < cur->data) { succ = cur; cur = cur->left; }
				else cur = cur->right;
			}
			return succ;
		}
		Node* predecessor_(const T& x) const { // largest < x
			Node* cur = root; Node* pred = nullptr;
			while (cur) {
				if (cur->data < x) { pred = cur; cur = cur->right; }
				else cur = cur->left;
			}
			return pred;
		}
		Node* lca_(Node* n, T a, T b) const {
			if (b < a) std::swap(a, b);
			while (n) {
				if (b < n->data) n = n->left;
				else if (n->data < a) n = n->right;
				else return n; // a <= n->data <= b
			}
			return nullptr;
		}
		Node* kth_smallest_(Node* n, std::size_t& k) const {
			if (!n) return nullptr;
			if (Node* L = kth_smallest_(n->left, k)) return L;
			if (--k == 0) return n;
			return kth_smallest_(n->right, k);
		}
		void print_range_(Node* n, const T& L, const T& R) const {
			if (!n) return;
			if (L < n->data) print_range_(n->left, L, R);
			if (!(n->data < L) && !(R < n->data)) std::cout << n->data << " ";
			if (n->data < R) print_range_(n->right, L, R);
		}

	public:
		Bst() = default;
		Bst(const Bst&) = delete;
		Bst& operator=(const Bst&) = delete;
		Bst(Bst&&) = delete;
		Bst& operator=(Bst&&) = delete;
		~Bst() { clear_(root); }

		// core ops
		void  insert(const T& x)      { root = insert_(root, x); }
		void  remove(const T& x)      { root = remove_(root, x); }
		Node* find (const T& x) const { return find_(root, x); }
		Node* findmin() const         { return findmin_(root); }
		Node* findmax() const         { return findmax_(root); }
		void  clear()                 { clear_(root); root = nullptr; }

		// traversals / checks
		void dfs_inorder()  const { dfs_inorder_(root);  std::cout << '\n'; }
		void dfs_preorder() const { dfs_preorder_(root); std::cout << '\n'; }
		void dfs_postorder() const{ dfs_postorder_(root);std::cout << '\n'; }
		void bfs_order()    const { bfs_order_(root);    std::cout << '\n'; }
		bool is_bst()       const { return is_bst_(root); }
		void print_leaves() const { print_leaves_(root); std::cout << '\n'; }
		void print_boundary()const{ print_boundary_(root); std::cout << '\n'; }

		// -------- Public wrappers for new utilities --------
		int   height()                  const { return height_(root); }
		bool  is_balanced()             const { return is_balanced_(root); }
		Node* lower_bound(const T& x)   const { return lower_bound_(root, x); }
		Node* upper_bound(const T& x)   const { return upper_bound_(root, x); }
		Node* successor(const T& x)     const { return successor_(x); }
		Node* predecessor(const T& x)   const { return predecessor_(x); }
		Node* lca(const T& a, const T& b) const { return lca_(root, a, b); }
		Node* kth_smallest(std::size_t k) const {
			return kth_smallest_(root, k);
		}
		void  print_range(const T& L, const T& R) const {
			print_range_(root, L, R); std::cout << '\n';
		}
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
