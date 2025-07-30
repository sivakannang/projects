#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <optional>

template<typename K, typename V, size_t N = 10 >
class HashTable {

	size_t count;
	std::vector<std::list<std::pair<K, V>>> table;

	size_t hash(const K& k) const {  return std::hash<K>()(k) % N ; }

	public:
		HashTable() : table(N), count(0) {}
		size_t size() const { return count; }

		void insert(const K& k, const V& v) {
			int idx = hash(k);
			for ( auto &it : table[idx] ) {
				if ( it.first == k ) {
					it.second = v;
					return;
				}
			}	
			table[idx].push_back(std::make_pair(k, v)); ++count;
		}

		bool remove(const K& k) {
			int idx = hash(k);
			for ( auto &it : table[hash(k)] ) {
				if ( it.first == k ) {
					table[hash(k)].erase(it);
					--count;
					return true;
				}
			}
			return false;
		}

		std::optional<V> find(const K& k) {
			int idx = hash(k);
			for ( auto &it : table[idx] ) {
				if ( it.first == k ) {
					return it.second;
				}
			}
			return std::nullopt;	
		}

		void print() {
			int bc = 0, lc = 0;
			for ( const auto& itv : table ) {
				lc = 0;
				for ( const auto& itl : itv ) {
					std::cout << "bucket[" << bc << "]" << itl.first << " " << itl.second << " | " ;
				}
				std::cout << std::endl;
				++bc;
			}
		}
};


int main()
{
	HashTable<int, std::string> htable;

	htable.insert(1, "kannan1");
	htable.insert(2, "kannan2");
	htable.insert(3, "kannan3");
	htable.insert(4, "kannan4");
	htable.insert(5, "kannan5");
	htable.insert(6, "kannan6");
	htable.insert(7, "kannan7");
	htable.insert(8, "kannan8");
	htable.insert(9, "kannan9");
	htable.insert(10, "kannan10");
	htable.insert(11, "kannan11");
	htable.insert(12, "kannan12");
	htable.insert(21, "kannan21");
	htable.insert(22, "kannan22");

	auto val = htable.find(5);
	if ( val )
		std::cout << *val << std::endl;

	htable.print();

	return 0;
}
