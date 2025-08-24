#include <iostream>
#include <vector>
#include <unordered_map>

vector<int> twoSum(const vector<int>& a, int T) {
    unordered_map<int,int> pos; // value -> index
    for (int i = 0; i < (int)a.size(); ++i) {
        if (auto it = pos.find(T - a[i]); it != pos.end())
            return {it->second, i};
        pos[a[i]] = i;
    }
    return {};
}


std::vector<std::pair<int,int>>
twoSumAll(const std::vector<int>& a, long long T) {
    std::unordered_map<long long, std::vector<int>> prev; // value -> all prior indices
    std::vector<std::pair<int,int>> out;

    for (int i = 0; i < (int)a.size(); ++i) {
        long long need = T - (long long)a[i];
        if (auto it = prev.find(need); it != prev.end()) {
            for (int j : it->second) out.emplace_back(j, i); // all pairs (j<i)
        }
        prev[(long long)a[i]].push_back(i);
    }
    return out; // O(n + P), where P is number of output pairs
}

