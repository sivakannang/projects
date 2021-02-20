
#include <iostream>
#include <cstdlib>
#include <ctime>

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
			if ( is_swapped == false ) break;
		}
	}

	// merge two sub arrays of arr[], first sub array is arr[l..m], second sub array is arr[m+1..r] 
	static void merge_sub(T arr[], int l, int m, int r) {
		
		int n1 = m - l + 1;
		int n2 = r -m;

		int L[n1], R[n2];

		// copy data's to temp arrays L[], R[]
		for ( int i = 0; i < n1; ++i )
			L[i] = arr[l+i];
		for ( int j = 0; j < n2; ++j )
			R[j] = arr[m+1+j];

		int i = 0;  // initial index of first sub array
		int j = 0;  // initial index of second sub array
		int k = l;  // initial index of merged sub array
	        
		while ( i < n1 && j < n2 )
			arr[k++] = ( L[i] < R[j] ) ? L[i++] : R[j++];

		while ( i < n1 )  // copy remaining elements of L[], if there are any
			arr[k++] = L[i++];

		while ( j < n2 )  // copy remaining elements of R[], if there are any
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

	static void generate(int arr[], int size, int mod) {
		for ( int i = 0; i < size; ++i )
			arr[i] = std::rand() % mod;

	}
};

int main() {

	int arr[10] {};
	int size = sizeof(arr) / sizeof(int);
	int mod = 100;

	std::srand((unsigned) time(NULL));

	Sort<int>::generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::selection(arr, size);
	Sort<int>::print(arr, size);

	Sort<int>::generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::bubble(arr, size);
	Sort<int>::print(arr, size);

	Sort<int>::generate(arr, size, mod);
	Sort<int>::print(arr, size);
	Sort<int>::merge(arr, 0, size-1);
	Sort<int>::print(arr, size);
}
