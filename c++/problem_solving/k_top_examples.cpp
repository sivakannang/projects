/*
  k_top_examples.cpp

  ============================================================
  OVERVIEW
  ------------------------------------------------------------
  A collection of "k-th" style utilities for arrays and BSTs:
    1) k smallest / k largest (with duplicates)
    2) k smallest / k largest DISTINCT values
    3) k most frequent values (deterministic tie-break)
    4) k-th smallest / largest in a BST

  Each function includes notes on approach, complexity, and edge cases.

  Build:
    g++ -std=c++17 -O2 -Wall -Wextra -pedantic k_top_examples.cpp -o ktop

  Run:
    ./ktop
*/

#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// Pretty-print helper for vectors
// ------------------------------------------------------------
template <typename T>
void print(const std::vector<T>& v, const std::string& header = "output") {
    std::cout << header << " : ";
    for (const auto& x : v) std::cout << x << ' ';
    std::cout << '\n';
}

// ============================================================
// 1) k SMALLEST (with duplicates)  — Max-heap of size k
// ------------------------------------------------------------
// Problem:
//   Return the k smallest elements from v (k may be <= v.size()).
//   Duplicates are allowed and preserved. Result is returned in
//   ascending order.
//
// Approach:
//   Maintain a max-heap (size ≤ k). Push elements; if heap grows
//   beyond k, pop the largest. At the end, the heap contains the
//   k smallest. Extract and reverse to get ascending order.
//
// Complexity: O(n log k) time, O(k) extra space.
//
// Edge cases:
//   - If k == 0 or v is empty → return empty.
//   - If k > v.size() → effectively returns all elements, sorted.
//
template <typename T>
std::vector<T> kth_smallest(const std::vector<T>& v, std::size_t k) {
    if (k == 0 || v.empty()) return {};
    std::priority_queue<T> pq; // max-heap
    pq = std::priority_queue<T>(); // ensure empty

    for (const T& x : v) {
        if (pq.size() < k) pq.push(x);
        else if (x < pq.top()) { pq.pop(); pq.push(x); }
    }
    std::vector<T> out(pq.size());
    std::size_t i = out.size();
    while (i--) { out[i] = pq.top(); pq.pop(); } // now ascending
    return out;
}

// ============================================================
// 2) k LARGEST (with duplicates)   — Min-heap of size k
// ------------------------------------------------------------
// Same reasoning as above, but mirrored.
//
// Complexity: O(n log k) time, O(k) space.
//
template <typename T>
std::vector<T> kth_largest(const std::vector<T>& v, std::size_t k) {
    if (k == 0 || v.empty()) return {};
    std::priority_queue<T, std::vector<T>, std::greater<T>> pq; // min-heap

    for (const T& x : v) {
        if (pq.size() < k) pq.push(x);
        else if (x > pq.top()) { pq.pop(); pq.push(x); }
    }
    std::vector<T> out(pq.size());
    std::size_t i = out.size();
    while (i--) { out[i] = pq.top(); pq.pop(); } // ascending of top-k largest
    return out;
}

// ============================================================
// 3) k SMALLEST DISTINCT values — std::set
// ------------------------------------------------------------
// Problem:
//   Return the k smallest DISTINCT values.
//
// Approach:
//   Keep a sorted set of size ≤ k. Insert each value; if size exceeds k,
//   erase the largest (prev(end())). The set holds the k smallest distinct.
//
// Complexity: O(n log k) time, O(k) space.
// Note: Requires T to be LessComparable (has operator<).
//
template <typename T>
std::vector<T> kth_smallest_distinct(const std::vector<T>& v, std::size_t k) {
    if (k == 0 || v.empty()) return {};
    std::set<T> s; // unique + sorted
    for (const T& x : v) {
        s.insert(x);
        if (s.size() > k) s.erase(std::prev(s.end())); // drop largest
    }
    return std::vector<T>(s.begin(), s.end()); // ascending
}

// ============================================================
// 4) k LARGEST DISTINCT values — std::set
// ------------------------------------------------------------
// Mirror of above: keep the k largest distinct values.
//
// Complexity: O(n log k) time, O(k) space.
//
template <typename T>
std::vector<T> kth_largest_distinct(const std::vector<T>& v, std::size_t k) {
    if (k == 0 || v.empty()) return {};
    std::set<T> s;
    for (const T& x : v) {
        s.insert(x);
        if (s.size() > k) s.erase(s.begin()); // drop smallest
    }
    // return in descending (largest→smallest) for readability
    return std::vector<T>(s.rbegin(), s.rend());
}

// ============================================================
// 5) k MOST FREQUENT values — deterministic tie-break
// ------------------------------------------------------------
// Problem:
//   Return the k most frequent values in v.
//
// Tie-break rule (deterministic):
//   If two values have the same frequency, the smaller value comes first.
//
// Approach A (deterministic, simple):
//   Build frequency map -> move to vector -> sort by
//   (freq DESC, value ASC) -> take first k.
//   Complexity: O(m log m) where m = number of distinct values.
//
// Approach B (heap, good when k << m, but tie-breaking is trickier):
//   Use a min-heap of size k keyed by (freq ASC, value DESC) so that the
//   "worst" item stays on top; replace when a better candidate arrives.
//   (We implement Approach A for clarity & fully deterministic behavior.)
//
template <typename T>
std::vector<T> kth_most_frequent(const std::vector<T>& v, std::size_t k) {
    if (k == 0 || v.empty()) return {};
    std::unordered_map<T, std::size_t> freq;
    freq.reserve(v.size());
    for (const auto& x : v) ++freq[x];

    std::vector<std::pair<T, std::size_t>> items;
    items.reserve(freq.size());
    for (const auto& kv : freq) items.emplace_back(kv.first, kv.second);

    // Sort by (frequency DESC, value ASC)
    std::sort(items.begin(), items.end(),
              [](const auto& a, const auto& b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first < b.first;
              });

    // Take first k (or fewer if distinct values < k)
    std::vector<T> out;
    const std::size_t take = std::min(k, items.size());
    out.reserve(take);
    for (std::size_t i = 0; i < take; ++i) out.push_back(items[i].first);
    return out;
}

// ============================================================
// 6) k-th SMALLEST / LARGEST in a BST  — iterative inorder
// ------------------------------------------------------------
// Problem:
//   Given a BST (Node<T> with `key`, `left`, `right`), return the
//   k-th smallest (or largest) key if it exists.
//
// Approach:
//   Inorder traversal is sorted for BSTs.
//   - k-th smallest: inorder (left, node, right).
//   - k-th largest : reverse inorder (right, node, left).
//   Stop once we count k nodes.
//
// Complexity: O(h + k) time, O(h) space (h = tree height).
//
template <typename T>
struct Node {
    T key{};
    Node *left{nullptr}, *right{nullptr};
};

template <typename T>
std::optional<T> kth_smallest(Node<T>* root, std::size_t k) {
    if (!root || k == 0) return std::nullopt;
    std::stack<Node<T>*> st;
    Node<T>* cur = root;
    std::size_t seen = 0;

    while (cur || !st.empty()) {
        while (cur) { st.push(cur); cur = cur->left; }
        cur = st.top(); st.pop();
        if (++seen == k) return cur->key;  // found
        cur = cur->right;
    }
    return std::nullopt; // k > node count
}

template <typename T>
std::optional<T> kth_largest(Node<T>* root, std::size_t k) {
    if (!root || k == 0) return std::nullopt;
    std::stack<Node<T>*> st;
    Node<T>* cur = root;
    std::size_t seen = 0;

    while (cur || !st.empty()) {
        while (cur) { st.push(cur); cur = cur->right; }
        cur = st.top(); st.pop();
        if (++seen == k) return cur->key;
        cur = cur->left;
    }
    return std::nullopt;
}

// -------------------------
// Demo / quick sanity tests
// -------------------------
int main() {
    {
        std::vector<int> v = {4, 5, 6, 1, 2, 2, 2};
        print(v, "array input");
        print(kth_smallest(v, 3),      "k_smallest_3");     // 3 smallest
        print(kth_largest(v, 3),       "k_largest_3");      // 3 largest
    }
    std::cout << '\n';

    {
        std::vector<int> v = {4, 1, -1, 2, -1, 2, 3};
        print(v, "array input");
        print(kth_smallest_distinct(v, 2), "k_smallest_distinct_2"); // {-1,1} or {-1,2} depending on data
        print(kth_largest_distinct(v, 2),  "k_largest_distinct_2");  // {4,3}
    }
    std::cout << '\n';

    {
        std::vector<int> v = {1,1,1,2,3,4,5,5,5,5};
        print(v, "array input");
        print(kth_most_frequent(v, 2), "k_most_frequent_2"); // {5,1} (freq: 4 and 3), tie-break by value
    }
    std::cout << '\n';

    // Optional: small BST example for kth-smallest/largest
    {
        //       5
        //     /   \
        //    3     8
        //   / \   / \
        //  2  4  7   9
        Node<int> n2{2}, n4{4}, n7{7}, n9{9};
        Node<int> n3{3, &n2, &n4};
        Node<int> n8{8, &n7, &n9};
        Node<int> n5{5, &n3, &n8};

        auto k1 = kth_smallest(&n5, 3); // 3rd smallest -> 4
        auto k2 = kth_largest (&n5, 2); // 2nd largest  -> 8
        std::cout << "BST kth_smallest(3): " << (k1 ? std::to_string(*k1) : "null") << "\n";
        std::cout << "BST kth_largest(2) : " << (k2 ? std::to_string(*k2) : "null") << "\n";
    }

    return 0;
}

