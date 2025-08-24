/*
   arrange_partition_examples.cpp

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

============================================================
PROBLEM C: sort012 — Dutch National Flag (only 0,1,2)
------------------------------------------------------------
INPUT:
- a: vector<int> containing only {0,1,2} (modified in place)

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

// ============================================================
// A) arrange_2: group all elements equal to `key` to the front
// ============================================================
void arrange_2(std::vector<int>& arr, int key) {
	// Two-pointer approach:
	// `left` scans from the beginning; `right` scans from the end.
	// - If arr[left] == key, it's already in the correct group -> left++
	// - Else if arr[right] == key, swap arr[left] and arr[right],
	//   then advance both (we've placed a key at `left`)
	// - Else (both arr[left] and arr[right] are not key), just move right--
	// Loop until left crosses right.
	int left = 0;
	int right = static_cast<int>(arr.size()) - 1;

	while (left < right) {
		if (arr[left] == key) {
			++left;
		} else if (arr[right] == key) {
			std::swap(arr[left++], arr[right--]);
		} else {
			--right;
		}
	}
	// After loop: all positions [0 .. left-1] are `key`.
	// The rest are non-key values.
}

// ==================================================================
// B) arrange_3: group `key1` first, then `key2`, then everything else
// ==================================================================
void arrange_3(std::vector<int>& arr, int key1 = 0, int key2 = 1) {
	// Phase 1: move all key1 to the front (like arrange_2)
	int left = 0;
	int right = static_cast<int>(arr.size()) - 1;
	while (left < right) {
		if (arr[left] == key1) {
			++left;
		} else if (arr[right] == key1) {
			std::swap(arr[left++], arr[right--]);
		} else {
			--right;
		}
	}
	// Now indices [0 .. left-1] are key1.

	// Phase 2: starting at `left`, move all key2 right after the key1 block.
	// Re-initialize right; keep left as the current start of "non-key1" region.
	right = static_cast<int>(arr.size()) - 1;
	while (left < right) {
		if (arr[left] == key2) {
			++left;
		} else if (arr[right] == key2) {
			std::swap(arr[left++], arr[right--]);
		} else {
			--right;
		}
	}
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
				// If array is guaranteed to have only {0,1,2}, this should not happen.
				// If other values can appear, decide how you want to handle them.
				++mid; // safe fallback: skip
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
		print(v, "arrange_2() output"); // expect all 0s first, then 1s
		std::cout << '\n';
	}
	{
		std::vector<int> v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
		print(v, "arrange_3() input");
		arrange_3(v, 0, 1);
		print(v, "arrange_3() output"); // expect 0s, then 1s, then the rest (2s here)
		std::cout << '\n';
	}
	{
		std::vector<int> v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
		print(v, "sort012() input");
		sort012(v);
		print(v, "sort012() output"); // expect sorted [0...1...2...]
		std::cout << '\n';
	}
	return 0;
}

