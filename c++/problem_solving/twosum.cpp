/*
g++ -std=c++17 -O2 -Wall -Wextra twosum.cpp -o twosum

Problem: Two Sum (single pair) and Two Sum (all pairs)

Given an array of integers `a` and a target sum `T` (type long):

1) two_sum(a, T) -> returns indices {i, j} (0-based, i < j) of any single pair
such that a[i] + a[j] == T. If none exists, returns {}.

2) two_sum_all(a, T) -> returns ALL index pairs (i, j) with i < j and
a[i] + a[j] == T.

Examples:
a = {2, 7, 11, 15}, T = 9
two_sum -> {0, 1}
two_sum_all -> {(0,1)}

a = {1, 1, 1}, T = 2
two_sum -> {0, 1} (any)
two_sum_all -> {(0,1), (0,2), (1,2)}

Complexity:
- two_sum:     O(n) time, O(n) space
- two_sum_all: O(n + P) time, O(n) space, where P is #pairs in output

`long` is 64-bit on 64-bit linux, so the sum of two ints can't overflow.

If the array is sorted, two pointers need only O(1) space :
  left = 0, right = n-1. If the sum is too small left++, if it is too big right--.
Sorting first costs O(n log n), and it loses the original indices, so a hash map is the usual choice when indices are needed.
*/

#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>   // std::pair
#include <cstddef>   // std::size_t

// Return any one pair of indices {i, j} (i < j) whose values sum to T.
// If not found, returns an empty vector.
std::vector<int> two_sum(const std::vector<int>& a, long T) {
	std::unordered_map<long, int> last_pos; // value -> last index seen
	last_pos.reserve(a.size());

	for (int i = 0; i < static_cast<int>(a.size()); ++i) {
		long ai = static_cast<long>(a[i]);
		long need = T - ai;

		if (auto it = last_pos.find(need); it != last_pos.end()) {
			return {it->second, i}; // i > it->second
		}
		last_pos[ai] = i;
	}
	return {};
}

// Return all pairs (i, j) with i < j and a[i] + a[j] == T.
std::vector<std::pair<int,int>>
two_sum_all(const std::vector<int>& a, long T) {
	std::unordered_map<long, std::vector<int>> prior; // value -> all prior indices
	prior.reserve(a.size());

	std::vector<std::pair<int,int>> out;
	out.reserve(a.size()); // heuristic

	for (int i = 0; i < static_cast<int>(a.size()); ++i) {
		long ai = static_cast<long>(a[i]);
		long need = T - ai;

		if (auto it = prior.find(need); it != prior.end()) {
			for (int j : it->second) out.emplace_back(j, i); // all j < i with a[j] == need
		}
		prior[ai].push_back(i);
	}
	return out; // O(n + P)
}

// ---- helpers for printing ----
template <typename T>
void print_vec(const std::vector<T>& v, const char* label = "") {
	if (label && *label) std::cout << label;
	std::cout << "[";
	for (std::size_t i = 0; i < v.size(); ++i) {
		std::cout << v[i] << (i + 1 < v.size() ? ", " : "");
	}
	std::cout << "]\n";
}

template <typename A, typename B>
void print_pairs(const std::vector<std::pair<A,B>>& vp, const char* label = "") {
	if (label && *label) std::cout << label;
	std::cout << "[";
	for (std::size_t i = 0; i < vp.size(); ++i) {
		std::cout << "(" << vp[i].first << "," << vp[i].second << ")"
			<< (i + 1 < vp.size() ? ", " : "");
	}
	std::cout << "]\n";
}

// Print both results, and compare them with the expected ones
void run_example(const char* title, const std::vector<int>& a, long T,
		const std::vector<int>& want_one, const std::vector<std::pair<int,int>>& want_all) {
	auto one = two_sum(a, T);
	auto all = two_sum_all(a, T);
	std::cout << title << "\n";
	print_vec(one, "two_sum -> ");
	print_pairs(all, "two_sum_all -> ");
	if (one != want_one) print_vec(want_one, "  MISMATCH, expected ");
	if (all != want_all) print_pairs(want_all, "  MISMATCH, expected ");
	std::cout << "\n";
}

// ---- demo ----
int main() {
	run_example("Example 1: a={2,7,11,15}, T=9", {2, 7, 11, 15}, 9, {0, 1}, {{0, 1}});
	run_example("Example 2: a={1,1,1}, T=2", {1, 1, 1}, 2, {0, 1}, {{0, 1}, {0, 2}, {1, 2}});
	run_example("Example 3: a={3,2,4}, T=6", {3, 2, 4}, 6, {1, 2}, {{1, 2}});
	run_example("Example 4: a={-2,-1,3,5,6,-4,9}, T=4", {-2, -1, 3, 5, 6, -4, 9}, 4, {1, 3}, {{1, 3}, {0, 4}});
	run_example("Example 5: no pair", {1, 2, 3}, 100, {}, {});
	run_example("Example 6: the sum doesn't fit in int", {2147483647, 2147483647}, 4294967294L, {0, 1}, {{0, 1}});
	return 0;
}

