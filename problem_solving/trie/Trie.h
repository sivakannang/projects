#pragma once
#include <string>
#include <iostream>

const int CHAR_SIZE = 128;

class Trie {

	Trie* children[CHAR_SIZE]{};
	bool is_leaf = false;

	void print(Trie* trie, char str[], int level);

public:

	void insert(std::string word);
	bool search(std::string word);
	void print();
};
