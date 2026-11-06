// g++ -std=c++17 sorting.cpp -o sorting
//
//              Best        Average     Worst       Extra space   Stable
// selection    O(n^2)      O(n^2)      O(n^2)      O(1)          No
// bubble       O(n)        O(n^2)      O(n^2)      O(1)          Yes
// merge        O(n log n)  O(n log n)  O(n log n)  O(n)          Yes
//
// Stable means equal elements keep their original order.

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

template<typename T>
class Sort {

public:
	static void selection(T t[], int size ) {

		for ( int i = 0; i < size-1; ++i ) {
			int min_idx = i;
			for ( int j = i+1; j < size; ++j )
				if ( t[j] < t[min_idx] )
					min_idx = j;
			std::swap( t[i], t[min_idx] );
		}
	}

	static void bubble(T t[], int size ) {
		for ( int i = 0; i < size-1; ++i ) {
			bool is_swapped = false;
			for ( int j = 0; j < size-1-i; ++j ) {
				if ( t[j] > t[j+1] ) {
				       std::swap(t[j], t[j+1]);
				       is_swapped = true;
				}
			}
			if ( is_swapped == false ) break;   // No swaps means it is already sorted
		}
	}

	// merge two sub arrays of arr[], first sub array is arr[l..m], second sub array is arr[m+1..r]
	static void merge_sub(T arr[], int l, int m, int r) {

		// Temp arrays of T. Arrays sized at runtime, like int L[n1], aren't standard C++.
		std::vector<T> L(arr + l, arr + m + 1);
		std::vector<T> R(arr + m + 1, arr + r + 1);

		std::size_t i = 0;  // initial index of first sub array
		std::size_t j = 0;  // initial index of second sub array
		int k = l;          // initial index of merged sub array

		// On a tie take from L, which keeps equal elements in order and makes the sort stable
		while ( i < L.size() && j < R.size() )
			arr[k++] = ( R[j] < L[i] ) ? R[j++] : L[i++];

		while ( i < L.size() )  // copy remaining elements of L[], if there are any
			arr[k++] = L[i++];

		while ( j < R.size() )  // copy remaining elements of R[], if there are any
			arr[k++] = R[j++];
	}

	static void merge(T arr[], int l, int r) {

		if ( l >= r ) return;

		int m = l + ( r - l ) / 2;

		merge(arr, l, m);
		merge(arr, m+1, r);

		merge_sub(arr, l, m, r);
	}

	static void print(T t[], int size) {
		for ( int i = 0 ; i < size; ++i )
			std::cout << t[i] << " " ;
		std::cout << std::endl;
	}
};

static void generate(int arr[], int size, int mod) {
	for ( int i = 0; i < size; ++i )
		arr[i] = std::rand() % mod;
}

int main() {

	int arr[10] {};
	int size = sizeof(arr) / sizeof(int);
	int mod = 100;

	std::srand((unsigned) std::time(nullptr));

	generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::selection(arr, size);
	Sort<int>::print(arr, size);

	generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::bubble(arr, size);
	Sort<int>::print(arr, size);

	generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::merge(arr, 0, size-1);
	Sort<int>::print(arr, size);

	double prices[] = {2.5, 1.5, 1.25, 3.75};
	Sort<double>::merge(prices, 0, 3);
	Sort<double>::print(prices, 4);
}
