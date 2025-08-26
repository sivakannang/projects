// filename: subarray_problems.cpp
// build:    g++ -std=c++17 -O2 -Wall -Wextra subarray_problems.cpp -o app

#include <bits/stdc++.h>
using namespace std;

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
class SubArray {
	public:
		static int longest_increasing_contiguous_subarray_length(const int a[], int n) {
			if (n <= 0) return 0;
			int max_len = 1, cur_len = 1;

			for (int i = 0; i < n - 1; ++i) {
				if (a[i] < a[i + 1]) {
					++cur_len;
				} else {
					max_len = std::max(max_len, cur_len);
					cur_len = 1;
				}
			}
			return std::max(max_len, cur_len);
		}
};

// ============================================================================
// PROBLEM B: Longest Strictly Increasing Contiguous Subarray (Indices)
//
// Statement:
//   Return the (start, end) indices of one longest strictly increasing contiguous
//   subarray. If multiple, return any. For n=0, return (-1,-1).
//
// Example:
//   [5,6,7,1,2,3,4] -> return (3,6) for subarray [1,2,3,4]
//
// Complexity:
//   Time O(n), Space O(1)
// ----------------------------------------------------------------------------
class SubArrayB {
	public:
		static std::pair<int, int> longest_increasing_contiguous_subarray_indices(const std::vector<int>& v)
		{
			std::pair<int, int> indices {0, 0};
			int size = v.size(), lIdx = 0, rIdx = 0;

			if ( size <= 1) { return indices; }

			for ( int i = 0; i < size-1; i++  ) {

				if ( v[i] < v[i+1] ) {
					rIdx = i + 1;
					if ( (rIdx - lIdx) > (indices.second - indices.first) )
						indices = {lIdx, rIdx};
				} else {
					lIdx = rIdx = i + 1;
				}
			}
			return indices;
		}
};

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
class SubArrayC {
	public:
		static int longest_subarray_with_consecutive_values_any_order_length(const int a[], int n) {
			if (n <= 0) return 0;
			int best = 1;

			for (int i = 0; i < n; ++i) {
				int curMin = a[i], curMax = a[i];
				unordered_set<int> seen;
				seen.insert(a[i]);

				for (int j = i + 1; j < n; ++j) {
					if (seen.find(a[j]) != seen.end()) break; // duplicate
					seen.insert(a[j]);
					curMin = min(curMin, a[j]);
					curMax = max(curMax, a[j]);

					int len = j - i + 1;
					if (curMax - curMin + 1 == len) {
						best = max(best, len);
					}
				}
			}
			return best;
		}
};

// ============================================================================
// PROBLEM D: First Non-Repeating Element
//
// Statement:
//   Return the first element (by array order) whose frequency is 1; return -1 if none.
//
// Example:
//   [2,3,4,2,3,5] -> 4
//   [1,1,2,2]     -> -1
//
// Complexity:
//   Time O(n), Space O(n)
// ----------------------------------------------------------------------------
class SubArrayD {
	public:
		static int first_non_repeating_element_in_array(const int a[], int n) {
			unordered_map<int, int> freq;
			freq.reserve(n * 2);
			for (int i = 0; i < n; ++i) ++freq[a[i]];
			for (int i = 0; i < n; ++i) if (freq[a[i]] == 1) return a[i];
			return -1;
		}
};

// ============================================================================
// Demo Helpers
// ----------------------------------------------------------------------------
template<typename T>
static void print_vec(const vector<T>& v, const string& name = "arr") {
	cout << name << " = [";
	for (size_t i = 0; i < v.size(); ++i) {
		if (i) cout << ", ";
		cout << v[i];
	}
	cout << "]\n";
}
static void print_subarray(const vector<int>& v, int L, int R, const string& label) {
	cout << label << ": ";
	if (L < 0 || R < 0 || L >= (int)v.size() || R >= (int)v.size() || L > R) {
		cout << "(none)\n";
		return;
	}
	cout << "L=" << L << ", R=" << R << ", [";
	for (int i = L; i <= R; ++i) {
		if (i > L) cout << ", ";
		cout << v[i];
	}
	cout << "]\n";
}

// ============================================================================
// Main: Quick Demonstration
// ----------------------------------------------------------------------------
int main() {
	cout << "===== Problem A =====\n";
	vector<int> v1{5, 6, 7, 1, 2, 3, 4};
	print_vec(v1, "v1");
	cout << "Length = " 
		<< SubArray::longest_increasing_contiguous_subarray_length(v1.data(), v1.size()) 
		<< " (expected 4)\n";

	cout << "\n===== Problem B =====\n";
	auto [L, R] = SubArrayB::longest_increasing_contiguous_subarray_indices(v1.data(), v1.size());
	print_subarray(v1, L, R, "Longest increasing run");

	cout << "\n===== Problem C =====\n";
	vector<int> v3{10, 12, 11};
	print_vec(v3, "v3");
	cout << "Length = " 
		<< SubArrayC::longest_subarray_with_consecutive_values_any_order_length(v3.data(), v3.size()) 
		<< " (expected 3)\n";

	cout << "\n===== Problem D =====\n";
	vector<int> v6{2, 3, 4, 2, 3, 5};
	print_vec(v6, "v6");
	cout << "First non-repeating = " 
		<< SubArrayD::first_non_repeating_element_in_array(v6.data(), v6.size()) 
		<< " (expected 4)\n";

	return 0;
}

