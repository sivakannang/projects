#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>


int f() {
	static int i = 0;
	return ++i % 3;
}

void arrange_2(int arr[], int len, int key) {

	int pos = 0;
	--len;
	while (pos < len) {

		if (arr[pos] == key) {
			++pos;
			continue;
		}
		else if (arr[pos] != key && arr[len] == key) {
			std::swap(arr[pos++], arr[len--]);
		}
		else { // both are not keys
			--len;
		}
	}
}

void arrange_3(int arr[], int length, int key1 = 0, int key2 = 1) {

	int pos = 0;
	int key = key1;
	int len = length - 1;

	while (pos < len) {

		if (arr[pos] == key) {
			++pos;
			continue;
		}
		else if (arr[pos] != key && arr[len] == key) {
			std::swap(arr[pos++], arr[len--]);
		}
		else { // both are not keys
			--len;
		}

		if ( pos >= len && key != key2 ) {
			key = key2;
			len = length-1;
		}
	}
}

void sort012(int a[], int arr_size)
{
	int high = arr_size - 1;
	int pos = 0;
	int low = 0;

	// Iterate till all the elements are sorted 
	while (pos <= high) {
		switch (a[pos]) {

		case 0:   // element is 0
			std::swap(a[low++], a[pos++]);
			break;

		case 1:  // If the element is 1
			pos++;
			break;

		case 2: // If the element is 2 
			std::swap(a[pos], a[high--]);
			break;
		}
	}
}

int find_missing_number(int ar[], int size) 
{ 
    int l = 0, r = size-1;
    int mid;
    while ( (r-l) > 1 ) {
        mid = (l+r) / 2;
	std::cout << " l : " << l << " , m : " << mid << " , r : " << r << " , arr[" << mid << "] = " << ar[mid] << std::endl;
        if ( ar[l]-l != ar[mid]-mid )
        {
            r = mid;
        }
        else if ( ar[r]-r != ar[mid]-mid)
        {
            l = mid;
        }
        else
        {
            //std::cout <<" l : " << l << " , m : " << m << " , r : " << r << std::endl;
        }
    }
    return (ar[mid] + 1); 
}

class Utility {
	public:
		static auto atm_denomination(long amt) {
			
			int notes[] = {2000, 1000, 500, 200, 100, 50, 20, 10};
			int idx = 0;
			int count = 0;
			std::vector<std::pair<int, int>> vec;

			while ( amt > 0 ) {
				count = amt / notes[idx];
				if ( count > 0 )
					vec.push_back({notes[idx], count});
				amt = amt % notes[idx++];
			}
			return vec;
		}

		static auto int_to_roman(int num) {
			
			std::string result;	
			
			std::string romans[] {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
			int numbers[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
			
			int size = sizeof(numbers) / sizeof(numbers[0]);

			for ( int idx = 0; idx < size; ++idx) {
				while ( num >= numbers[idx] ) {
					result += romans[idx];
					num -= numbers[idx];
				}
			}
			return result;
		}

		static auto roman_to_int(std::string& roman) {
			int result = 0;
		}

		template<typename T>
		static std::vector<T> find_dup( std::vector<T>& vec ) {

			std::unordered_set<T> uset;
			std::vector<T> v;

			for ( auto t : vec ) {
				if ( uset.find(t) == uset.end() )
					uset.insert(t);
				else
					v.push_back(t);
			}
			return v;
		}
};

class SubArray {
	public:
		static int longest_subarray_with_contigious_elements( int a[], int n ) {
			
			int max_count = 1;
			int cur_count = 1;

			for ( int i = 0; i < n-1; ++i )
			{
				if ( a[i] < a[i+1] )
					++cur_count;
				else {
					max_count = std::max(max_count, cur_count);
					cur_count = 1;
				}
			}
			return std::max(max_count, cur_count);
		}

		// need to implement
		static void longest_increasing_subarray( int a[], int n ) {
		}

		static int first_non_repeating_element_in_array ( int a[], int n ) {

			std::unordered_map<int, int> umap;
			
			for ( int i = 0; i < n; ++i)
				++umap[a[i]];
			
			for ( int i = 0; i < n; ++i)
				if ( umap[a[i]] == 1 )
					return a[i];
			return -1;
		}
};


int main() {
	
	//int arr[] = { 0, 1, 0, 1, 1, 1, 0, 1, 0, 0 };
	//int arr[] = { 1, 1, 0, 2, 2, 1, 0, 1, 2, 0 };
	int arr[100000];

	std::generate(std::begin(arr), std::end(arr), f);
	auto t1 = std::chrono::high_resolution_clock::now();
	sort012(arr, 100000);
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << (t2 - t1).count() << std::endl;

	std::generate(std::begin(arr), std::end(arr), f);
	t1 = std::chrono::high_resolution_clock::now();
	arrange_3(arr, 100000);
	t2 = std::chrono::high_resolution_clock::now();
	std::cout << (t2 - t1).count() << std::endl;

	std::for_each(std::begin(arr), std::end(arr), [](auto &a) {
		std::cout << a << " ";
		});
	std::cout << std::endl;

	std::cout << "siv --> " << Utility::int_to_roman(120) << std::endl;

	return 0;
}
