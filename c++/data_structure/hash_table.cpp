
//https://www.hackerearth.com/practice/data-structures/hash-tables/basics-of-hash-tables/tutorial/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

void heap_test();

int hashtable_size = 20;

struct Student {
	int rollno;
	string name;
};

class HashTable {

	private:
		vector<Student> v[20];
		size_t hashfunc(string s);
	public:
		void insert(Student student);
		Student find(string name);

};

void HashTable::insert(Student student) {

	size_t hash_idx = hashfunc(student.name);
	v[hash_idx].push_back(student);
}

Student HashTable::find(string name) {

	Student student = {0};
	size_t hash_idx = hashfunc(name);
	if ( v[hash_idx].size() == 0 ) {
		cout << "data not found with key : " << name << endl;
		return student;

	}

	auto it = v[hash_idx].begin();
	while ( it != v[hash_idx].end() ) {
		if ( it->name == name) {
				cout << "data found with key : " << name << " => " << it->rollno << " " << it->name << endl;
				return *it;
		}
		it++;
	}

	cout << "data not found with key : " << name << endl;

	return student;

}

size_t HashTable::hashfunc(string s) {

	size_t value = 0;
	size_t hash_idx = 0;

	for ( int i = 0; i < s.length(); i++ )
		value += s[i];

	hash_idx = value % hashtable_size;
	cout << "hash_idx for " << s << " : " << hash_idx << endl;
	return hash_idx;
}

int main() {

	{
		heap_test();
		return 0;
	}

	string s;

	HashTable hashtable;

	int choice = 0;
	do {
		Student student = {0};
		cout << " ------------------- Choices ----------------- " << endl;
		cout << " 1. Insert" << endl;
		cout << " 2. Search" << endl;
		cout << " 3. Exit" << endl;
		cout << " Enter ur choice : ";
		cin >> choice;

		switch(choice) {
			case 1:
				cout << "Enter rollnumber and student name : ";
				cin >> student.rollno >> student.name;
				hashtable.insert(student);
				break;

			case 2:
				cout << "Enter rollnumber to search : ";
				cin >> s;
				student = hashtable.find(s);
				cout << " student details : " << student.rollno << " " << student.name << endl;
				break;
			case 3: default:
				return 0;

		}
	} while ( choice != 3);

	return 0;
}


void print_array(auto &arr){
	for ( auto &e : arr )
		std::cout << e << " ";
	std::cout << std::endl;
}




