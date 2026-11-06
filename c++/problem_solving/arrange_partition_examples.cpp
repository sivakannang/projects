/*
   g++ -std=c++17 -O2 -Wall -Wextra arrange_partition_examples.cpp -o arrange

   ============================================================
   PROBLEM A: arrange_2 — Group all occurrences of a key to the front
   ------------------------------------------------------------
INPUT:
- arr: vector<int> (modified in place)
- key: int (the value to move to the front)

GOAL:
Reorder arr so that all elements equal to `key` appear first,
followed by all other elements (relative order not required).

EXAMPLE:
arr = [0,1,0,1,1,1,0,1,0,0], key = 0
output could be: [0,0,0,0,0, 1,1,1,1,1]

NOTES:
- Single pass, two pointers: O(n) time, O(1) extra space.
- Not stable (relative order inside groups can change).

============================================================
PROBLEM B: arrange_3 — Group key1 first, then key2, then the rest
------------------------------------------------------------
INPUT:
- arr: vector<int> (modified in place)
- key1: int (first group), key2: int (second group)

GOAL:
Reorder arr so that all `key1` come first, followed by all `key2`,
then everything else after them (others can be any value).

EXAMPLE:
arr = [1,1,0,2,2,1,0,1,2,0], key1=0, key2=1
output could be: [0,0,0, 1,1,1,1, 2,2,2]

APPROACH:
- Do one partition pass to pull `key1` to the front.
- Then a second partition pass (starting from where we stopped)
to pull `key2` right after the `key1` block.
- O(n) time, O(1) space, not stable.
- The trap: when the pointers meet, that last element is not checked yet.
If it is `key1` and the second pass starts on it, it gets swapped out of
the front block. Example: {1,0,0} would become {0,1,0}.

============================================================
PROBLEM C: sort012 — Dutch National Flag (only 0,1,2)
------------------------------------------------------------
INPUT:
- a: vector<int> containing only {0,1,2} (modified in place)
Any other value is a caller error, the result isn't sorted then.

GOAL:
Sort the array in non-decreasing order using one pass and O(1) space.

EXAMPLE:
a = [1,1,0,2,2,1,0,1,2,0]  ->  [0,0,0,1,1,1,1,2,2,2]

INVARIANTS (classic DNF):
- [0 .. low-1]     == 0
- [low .. mid-1]   == 1
- [mid .. high]    == unknown (to be processed)
- [high+1 .. n-1]  == 2
*/

#include <cassert>
#include <iostream>
#include <vector>
#include <utility>   // std::swap
#include <string>

// ------------ UTIL: pretty print ------------
template<typename T>
void print(const T& t, const std::string& header = "array") {
	std::cout << header << " : ";
	for (const auto& x : t) std::cout << x << ' ';
	std::cout << '\n';
}

// ------------ UTIL: print the result, and compare it with the expected one ------------
void check(const std::vector<int>& got, const std::vector<int>& want, const std::string& header) {
	print(got, header);
	if (got != want) print(want, "  MISMATCH, expected");
}

// ============================================================
// Move all elements equal to `key` in arr[start .. n-1] to the front of that range.
// Returns the index just after the key group.
// ============================================================
int group_to_front(std::vector<int>& arr, int start, int key) {
	// Two-pointer approach:
	// `left` scans from the start; `right` scans from the end.
	// - If arr[left] == key, it's already in the correct group -> left++
	// - Else if arr[right] == key, swap arr[left] and arr[right],
	//   then advance both (we've placed a key at `left`)
	// - Else (both arr[left] and arr[right] are not key), just move right--
	// Loop until left reaches right.
	int n = static_cast<int>(arr.size());
	int left = start;
	int right = n - 1;

	while (left < right) {
		if (arr[left] == key) {
			++left;
		} else if (arr[right] == key) {
			std::swap(arr[left++], arr[right--]);
		} else {
			--right;
		}
	}

	// The element where the pointers met isn't checked yet
	if (left < n && arr[left] == key) {
		++left;
	}
	return left;   // [start .. left-1] are `key`, the rest are non-key values
}

// ============================================================
// A) arrange_2: group all elements equal to `key` to the front
// ============================================================
void arrange_2(std::vector<int>& arr, int key) {
	group_to_front(arr, 0, key);
}

// ==================================================================
// B) arrange_3: group `key1` first, then `key2`, then everything else
// ==================================================================
void arrange_3(std::vector<int>& arr, int key1 = 0, int key2 = 1) {
	int next = group_to_front(arr, 0, key1);
	group_to_front(arr, next, key2);
	// Result: [ key1... ][ key2... ][ everything else ... ]
}

// ============================================================
// C) sort012: Dutch National Flag (array contains only 0/1/2)
// ============================================================
void sort012(std::vector<int>& a) {
	// Invariants:
	// [0 .. low-1]   == 0
	// [low .. mid-1] == 1
	// [mid .. high]  == unknown
	// [high+1 .. n-1]== 2
	int n = static_cast<int>(a.size());
	int low = 0, mid = 0, high = n - 1;

	while (mid <= high) {
		switch (a[mid]) {
			case 0:
				std::swap(a[low++], a[mid++]);
				break;
			case 1:
				++mid;
				break;
			case 2:
				std::swap(a[mid], a[high--]); // do not mid++ here, re-check swapped-in element
				break;
			default:
				assert(false && "sort012 takes only 0, 1 and 2");
				++mid; // Don't loop forever in a release build. The result isn't sorted.
		}
	}
}

// -------------------------
// Demo / quick sanity tests
// -------------------------
int main() {
	{
		std::vector<int> v = {0, 1, 0, 1, 1, 1, 0, 1, 0, 0};
		print(v, "arrange_2() input");
		arrange_2(v, 0);
		check(v, {0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, "arrange_2() output");
		std::cout << '\n';
	}
	{
		std::vector<int> v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
		print(v, "arrange_3() input");
		arrange_3(v, 0, 1);
		check(v, {0, 0, 0, 1, 1, 1, 1, 2, 2, 2}, "arrange_3() output");
		std::cout << '\n';
	}
	{
		std::vector<int> v = {1, 0, 0};    // The pointers meet on a key1
		print(v, "arrange_3() input");
		arrange_3(v, 0, 1);
		check(v, {0, 0, 1}, "arrange_3() output");
		std::cout << '\n';
	}
	{
		std::vector<int> v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
		print(v, "sort012() input");
		sort012(v);
		check(v, {0, 0, 0, 1, 1, 1, 1, 2, 2, 2}, "sort012() output");
		std::cout << '\n';
	}
	return 0;
}

