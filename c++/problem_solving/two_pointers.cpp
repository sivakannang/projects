#include <iostream>
#include <vector>

void arrange_2(std::vector<int>& arr, int key) {

	int pos = 0;
	int len = arr.size()-1;
	while (pos < len) {

		if (arr[pos] == key) {
			++pos;
		}
		else if (arr[pos] != key && arr[len] == key) {
			std::swap(arr[pos++], arr[len--]);
		}
		else { // both are not keys
			--len;
		}
	}
}

void arrange_3(std::vector<int>& arr, int key1 = 0, int key2 = 1) {

	int pos = 0;
	int key = key1;
	int len = arr.size() - 1;

	while (pos < len) {

		if (arr[pos] == key) {
			++pos;
		}
		else if (arr[pos] != key && arr[len] == key) {
			std::swap(arr[pos++], arr[len--]);
		}
		else { // both are not keys
			--len;
		}

		if ( pos >= len && key != key2 ) {
			key = key2;
			len = arr.size()-1;
		}
	}
}

void sort012(std::vector<int> &a)
{
	int high = a.size() - 1;
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

template<typename T>
void print(const T& t, std::string header = "header") {
	std::cout << header << " : ";
	for ( const auto& x : t ) { std::cout << x << " ";}
	std::cout << std::endl;
}


int main() {

	std::vector<int> v;
       
	v = { 0, 1, 0, 1, 1, 1, 0, 1, 0, 0 };
	print(v, "arrange_2() input");
	arrange_2(v, 0);
	print(v, "arrange_2() output");
	
	v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
	print(v, "arrange_3() input");
	arrange_3(v, 0, 1);
	print(v, "arrange_3() output");

	v = {1, 1, 0, 2, 2, 1, 0, 1, 2, 0};
	print(v, "sort012() input");
	sort012(v);
	print(v, "sort012() output");

	return 0;
}
