#include <iostream>
#include <vector>
#include <algorithm>

static void print_vector(std::vector<int>& vec) {
	for (auto v : vec)
		std::cout << v << " ";
	std::cout << std::endl;
}

static void print_vectors(std::vector<std::vector<int>> &vec) {
	for (auto& v : vec)
		print_vector(v);
}

std::vector<int> vec_concat(std::vector<int>& v1, std::vector<int>&& v2) {
	std::vector<int> v(v1);
	for (auto& i : v2)
		v.push_back(i);
	return v;
}

void permutate_str(std::string a, int l, int r)
{
	// Base case  
	if (l == r)
		std::cout << a << std::endl;
	else
	{
		// Permutations made  
		for (int i = l; i <= r; i++)
		{

			// Swapping done  
			std::swap(a[l], a[i]);

			// Recursion called  
			permutate_str(a, l + 1, r);

			//backtrack  
			std::swap(a[l], a[i]);
		}
	}
}

void permutate_str(std::string str) {

	std::sort(str.begin(), str.end());

	do {
		std::cout << str << std::endl;
	} while (std::next_permutation(str.begin(), str.end()));
}

void permutate_array_of_array(std::vector<std::vector<int>>& ip, std::vector<std::vector<int>>& op, std::vector<int> current, uint16_t depth) {

	std::vector<std::vector<int>> op_t;
	if (depth == ip.size()) {
		op.emplace_back(current);
		return;
	}

	for (uint16_t i = 0; i < ip.at(depth).size(); i++) {
		permutate_array_of_array(ip, op, vec_concat(current, { ip.at(depth).at(i) }), depth + 1);
	}
}

std::vector<std::vector<int>> permutate_array_of_array(std::vector<std::vector<int>>& ip, std::vector<int> current, uint16_t depth) {

	std::vector<std::vector<int>> op_t;
	if (depth == ip.size()) {
		op_t.emplace_back(current);
		return op_t;
	}

	for (uint16_t i = 0; i < ip.at(depth).size(); i++) {
		for (auto &v : permutate_array_of_array(ip, vec_concat(current, { ip.at(depth).at(i) }), depth + 1))
			op_t.emplace_back(v);
	}

	return op_t;
}

std::vector<std::string> permutate_char_array(std::string str, int length) {
	if (length == 0) {
		return std::vector<std::string>{};
	}
	std::vector<std::string> results;
	std::vector<std::string> children = permutate_char_array(str, length - 1);
	for (auto letter : str ) {
		results.emplace_back(1, letter);
		for (auto child : children) {
			results.push_back(std::string(1, letter) + child);
		}
	}
	return results;
}

void test_array_of_array_permutate() {
	std::vector<std::vector<int>> ip{ {1, 2, 3}, {4, 5}, {6, 7}, {8, 9} };
	print_vectors(ip);
	std::cout << "----" << std::endl;
	std::vector<std::vector<int>> op = std::move(permutate_array_of_array(ip, {}, 0));
	print_vectors(op);
}

std::vector<int> get_char_indexes(std::string& s, char c) {
	uint16_t idx = 0;
	std::vector<int> v;
	for (auto& ch : s) {
		if (ch == c)
			v.push_back(idx);
		++idx;
	}
	return v;
}

void test_string_index_find_from_char_array(std::string str, std::string key) {

	std::vector<std::vector<int>> ip;
	std::vector<std::vector<int>> op;
	std::vector<std::vector<int>> s_op;
	for (auto& ch : key)
		ip.emplace_back(get_char_indexes(str, ch));

	permutate_array_of_array(ip, op, {}, 0);

	print_vectors(ip);
	std::cout << "---" << std::endl;
	for_each(op.begin(), op.end(), [&s_op](auto& v) {
		if (std::is_sorted(v.begin(), v.end()))
			s_op.emplace_back(v);
		});

	print_vectors(s_op);
}

void test_permutate_char_array() {
	auto results = std::move(permutate_char_array("ab", 2));
	for (auto s : results) std::cout << s << std::endl;
}


int main() {
	test_array_of_array_permutate();
	test_string_index_find_from_char_array("this string here is dummy", "this");
	permutate_str("ABC");
	test_permutate_char_array();

	return 0;
}