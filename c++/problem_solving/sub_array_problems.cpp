// build:    g++ -std=c++17 -O2 -Wall -Wextra sub_array_problems.cpp -o sub_array_problems

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ============================================================================
// PROBLEM A: Longest Strictly Increasing Contiguous Subarray (Length)
//
// Statement:
//   Given an integer array a[0..n-1], find the length of the longest subarray
//   consisting of strictly increasing consecutive positions (i.e., a[i] < a[i+1]).
//
// Example:
//   [5, 6, 7, 1, 2, 3, 4] -> longest increasing run is [1,2,3,4], length = 4
//
// Complexity:
//   Time O(n), Space O(1)
// ----------------------------------------------------------------------------
int longest_increasing_contiguous_subarray_length(const std::vector<int>& a) {
	if (a.empty()) return 0;
	int max_len = 1, cur_len = 1;

	for (std::size_t i = 0; i + 1 < a.size(); ++i) {
		if (a[i] < a[i + 1]) {
			++cur_len;
		} else {
			max_len = std::max(max_len, cur_len);
			cur_len = 1;
		}
	}
	return std::max(max_len, cur_len);
}

// ============================================================================
// PROBLEM B: Longest Strictly Increasing Contiguous Subarray (Indices)
//
// Statement:
//   Return the (start, end) indices of one longest strictly increasing contiguous
//   subarray. If multiple, return the first. For an empty array, return (-1,-1).
//
// Example:
//   [5,6,7,1,2,3,4] -> return (3,6) for subarray [1,2,3,4]
//
// Complexity:
//   Time O(n), Space O(1)
// ----------------------------------------------------------------------------
std::pair<int, int> longest_increasing_contiguous_subarray_indices(const std::vector<int>& v) {
	if (v.empty()) return {-1, -1};

	std::pair<int, int> indices {0, 0};
	int size = static_cast<int>(v.size()), lIdx = 0;

	for (int i = 0; i < size - 1; ++i) {
		if (v[i] < v[i + 1]) {
			int rIdx = i + 1;
			if ((rIdx - lIdx) > (indices.second - indices.first))
				indices = {lIdx, rIdx};
		} else {
			lIdx = i + 1;
		}
	}
	return indices;
}

// ============================================================================
// PROBLEM C: Longest Subarray With Contiguous Elements (Consecutive Integers)
//
// Statement:
//   Find the length of the longest subarray such that the set of its elements are
//   consecutive integers (max - min + 1 == length) and there are no duplicates.
//
// Example:
//   [10, 12, 11] -> length = 3 (elements {10,11,12})
//   [1, 2, 2, 3] -> length = 2 (e.g., [1,2] or [2,3])
//
// Complexity:
//   Time O(n^2), Space O(n)
// ----------------------------------------------------------------------------
int longest_subarray_with_consecutive_values_any_order_length(const std::vector<int>& a) {
	if (a.empty()) return 0;
	int n = static_cast<int>(a.size());
	int best = 1;

	for (int i = 0; i < n; ++i) {
		int curMin = a[i], curMax = a[i];
		std::unordered_set<int> seen;
		seen.insert(a[i]);

		for (int j = i + 1; j < n; ++j) {
			if (!seen.insert(a[j]).second) break; // duplicate
			curMin = std::min(curMin, a[j]);
			curMax = std::max(curMax, a[j]);

			int len = j - i + 1;
			// long long, as max - min can overflow int with values far apart
			if (static_cast<long long>(curMax) - curMin + 1 == len) {
				best = std::max(best, len);
			}
		}
	}
	return best;
}

// ============================================================================
// PROBLEM D: First Non-Repeating Element
//
// Statement:
//   Return the first element (by array order) whose frequency is 1, or nullopt if none.
//   A value such as -1 can't mean "none", as -1 can be in the array.
//
// Example:
//   [2,3,4,2,3,5] -> 4
//   [1,1,2,2]     -> nullopt
//
// Complexity:
//   Time O(n), Space O(n)
// ----------------------------------------------------------------------------
std::optional<int> first_non_repeating_element_in_array(const std::vector<int>& a) {
	std::unordered_map<int, int> freq;
	freq.reserve(a.size());
	for (int x : a) ++freq[x];
	for (int x : a) if (freq[x] == 1) return x;
	return std::nullopt;
}

// ============================================================================
// Demo Helpers
// ----------------------------------------------------------------------------
template<typename T>
static void print_vec(const std::vector<T>& v, const std::string& name = "arr") {
	std::cout << name << " = [";
	for (std::size_t i = 0; i < v.size(); ++i) {
		if (i) std::cout << ", ";
		std::cout << v[i];
	}
	std::cout << "]\n";
}

static void print_subarray(const std::vector<int>& v, int L, int R, const std::string& label) {
	std::cout << label << ": ";
	if (L < 0 || R < 0 || L >= static_cast<int>(v.size()) || R >= static_cast<int>(v.size()) || L > R) {
		std::cout << "(none)\n";
		return;
	}
	std::cout << "L=" << L << ", R=" << R << ", [";
	for (int i = L; i <= R; ++i) {
		if (i > L) std::cout << ", ";
		std::cout << v[i];
	}
	std::cout << "]\n";
}

// Print a result, and compare it with the expected one
template<typename T>
static void check(const std::string& label, const T& got, const T& want) {
	std::cout << label << " = " << got;
	if (got != want) std::cout << "  MISMATCH, expected " << want;
	std::cout << "\n";
}

static void check_first_non_repeating(const std::vector<int>& v, std::optional<int> want) {
	auto got = first_non_repeating_element_in_array(v);
	print_vec(v, "v");
	std::cout << "First non-repeating = " << (got ? std::to_string(*got) : "none");
	if (got != want) std::cout << "  MISMATCH";
	std::cout << "\n";
}

// ============================================================================
// Main: Quick Demonstration
// ----------------------------------------------------------------------------
int main() {
	std::cout << "===== Problem A =====\n";
	std::vector<int> v1{5, 6, 7, 1, 2, 3, 4};
	print_vec(v1, "v1");
	check("Length", longest_increasing_contiguous_subarray_length(v1), 4);

	std::cout << "\n===== Problem B =====\n";
	auto [L, R] = longest_increasing_contiguous_subarray_indices(v1);
	print_subarray(v1, L, R, "Longest increasing run");
	check("L", L, 3);
	check("R", R, 6);

	auto [emptyL, emptyR] = longest_increasing_contiguous_subarray_indices({});
	print_subarray({}, emptyL, emptyR, "Empty array");
	check("L", emptyL, -1);
	check("R", emptyR, -1);

	std::cout << "\n===== Problem C =====\n";
	std::vector<int> v3{10, 12, 11};
	print_vec(v3, "v3");
	check("Length", longest_subarray_with_consecutive_values_any_order_length(v3), 3);

	std::vector<int> v4{1, 2, 2, 3};
	print_vec(v4, "v4");
	check("Length", longest_subarray_with_consecutive_values_any_order_length(v4), 2);

	std::cout << "\n===== Problem D =====\n";
	check_first_non_repeating({2, 3, 4, 2, 3, 5}, 4);
	check_first_non_repeating({-1, 2, 2}, -1);
	check_first_non_repeating({2, 2}, std::nullopt);

	return 0;
}
