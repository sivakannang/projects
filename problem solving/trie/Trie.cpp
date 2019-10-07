#include "Trie.h"
#include <vector>

void Trie::insert(std::string word) {

	Trie* cur = this;

	for (auto letter : word) {
		if (cur->children[letter] == nullptr)
			cur->children[letter] = new Trie;
		cur = cur->children[letter];
	}
	cur->is_leaf = true;
}

bool Trie::search(std::string word) {

	Trie* cur = this;

	for (auto letter : word) {
		if (cur->children[letter] == nullptr)
			return false;
		cur = cur->children[letter];
	}

	return cur && cur->is_leaf;
}

void Trie::print(Trie* trie, char str[], int level) {

	if (trie->is_leaf) {
		str[level] = 0;
		std::cout << str << std::endl;
	}

	for (int i = 0; i < CHAR_SIZE; i++) {
		if (trie->children[i]) {
			str[level] = i;
			print(trie->children[i], str, level + 1);
		}
	}

}

void Trie::print() {
	char str[256]{};
	print(this, str, 0);
}

int main() {

	Trie trie;

	trie.insert("cat");
	trie.insert("dog");
	trie.insert("kanna");
	trie.insert("siva");
	trie.insert("ganesh");
	trie.insert("lion");
	trie.insert("king");
	trie.insert("queen");
	trie.print();

	std::vector<std::string> v = { "cat", "lion", "forest", "queen", "king", "baby" };
	
	for (auto& s : v)
		std::cout << s << " -> " << std::boolalpha << trie.search(s) << std::endl;

	return 0;
}