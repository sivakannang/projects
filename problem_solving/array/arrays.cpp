#include <iostream>
#include <algorithm>
#include <chrono>


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
        cout << " l : " << l << " , m : " << mid << " , r : " << r << " , arr[" << mid << "] = " << ar[mid] << endl;
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
            //cout <<" l : " << l << " , m : " << m << " , r : " << r << endl;
        }
    }
    return (ar[mid] + 1); 
}

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

	return 0;
}
