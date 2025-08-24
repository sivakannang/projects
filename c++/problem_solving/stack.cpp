// filename: stacks_paren_minmax.cpp
// build:    g++ -std=c++17 -O2 -Wall -Wextra stacks_paren_minmax.cpp -o app

#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <optional>
#include <utility>

// -----------------------------------------------------------------------------
// PROBLEM 1: Balanced Parentheses
//
// Statement:
//   Given a string containing '(', ')', '{', '}', '[' and ']', determine if the
//   brackets are balanced: every opening has a matching closing of the same
//   type, and nesting order is correct.
//
// Examples:
//   ""           -> true
//   "()"         -> true
//   "()[]{}"     -> true
//   "(]"         -> false
//   "([)]"       -> false
//   "{[]}"       -> true
//   "abc(def)"   -> true   (non-bracket chars are ignored)
//
// Complexity:
//   Time O(n), Space O(n) worst-case.
// -----------------------------------------------------------------------------
bool is_paranthesis_balanced(const std::string& str) {
	std::stack<char> s;

	for (char c : str) {
		switch (c) {
			case '{': case '(': case '[':
				s.push(c);
				break;

			case '}':
				if (s.empty() || s.top() != '{') return false;
				s.pop();
				break;
			case ']':
				if (s.empty() || s.top() != '[') return false; // <-- fix typo if copying manually
				s.pop();
				break;
			case ')':
				if (s.empty() || s.top() != '(') return false;
				s.pop();
				break;

			default: // ignore non-bracket characters
				break;
		}
	}
	return s.empty();
}

// -----------------------------------------------------------------------------
// PROBLEM 2: Stack with O(1) min() and max()
//
// Statement:
//   Design a stack supporting push/pop/top and retrieving current minimum and
//   maximum in O(1) time, correctly handling duplicates of minima/maxima.
//
// Approach:
//   Maintain three stacks:
//     - stack_ : actual elements
//     - mins_  : monotonic (<=) minima history
//     - maxs_  : monotonic (>=) maxima history
//
// API:
//   - void push(const T&)
//   - void pop()                    // safe no-op if empty (demo choice)
//   - std::optional<T> top()  const // nullopt if empty
//   - std::optional<T> min()  const // nullopt if empty
//   - std::optional<T> max()  const // nullopt if empty
//   - bool empty() const, size() const
//
// Complexity: O(1) per op; Space O(n).
// -----------------------------------------------------------------------------
template <typename T>
class stack_minmax {
	std::stack<T> stack_;
	std::stack<T> mins_;
	std::stack<T> maxs_;

	public:
	bool empty() const noexcept { return stack_.empty(); }
	std::size_t size() const noexcept { return stack_.size(); }

	void push(const T& value) {
		stack_.push(value);
		if (mins_.empty() || value <= mins_.top()) mins_.push(value);
		if (maxs_.empty() || value >= maxs_.top()) maxs_.push(value);
	}

	void pop() {
		if (stack_.empty()) return;           // demo-safe; prod might assert/throw
		const T v = stack_.top();
		stack_.pop();
		if (!mins_.empty() && v == mins_.top()) mins_.pop();
		if (!maxs_.empty() && v == maxs_.top()) maxs_.pop();
	}

	std::optional<T> top() const {
		if (stack_.empty()) return std::nullopt;
		return stack_.top();
	}
	std::optional<T> min() const {
		if (mins_.empty()) return std::nullopt;
		return mins_.top();
	}
	std::optional<T> max() const {
		if (maxs_.empty()) return std::nullopt;
		return maxs_.top();
	}
};

// ---------- pretty printers ----------
template<typename T>
static void print_opt(const std::string& name, const std::optional<T>& v) {
	std::cout << name << " = ";
	if (v) std::cout << *v << '\n';
	else   std::cout << "(nullopt)\n";
}

// ---------- Demonstration & Tests ----------
int main() {
	std::cout << "===== Problem 1: Balanced Parentheses =====\n";
	const std::vector<std::pair<std::string, bool>> paren_tests{
		{"", true},
			{"()", true},
			{"()[]{}", true},
			{"(]", false},
			{"([)]", false},
			{"{[]}", true},
			{"(((", false},
			{"abc(def)", true},
			{"[a+(b*c)-{d/e}]", true},
			{"[a+(b*c)-{d/e})", false},
			{"no brackets here", true},
	};
	for (const auto& [s, expected] : paren_tests) {
		bool got = is_paranthesis_balanced(s);
		std::cout << "Input: \"" << s << "\" -> " << (got ? "balanced" : "NOT balanced");
		if (got != expected) std::cout << "  [EXPECTED " << (expected ? "balanced" : "NOT balanced") << "]";
		std::cout << '\n';
	}

	std::cout << "\n===== Problem 2: Min-Max Stack =====\n";
	stack_minmax<int> smm;

	std::cout << "Push sequence: 5, 3, 7, 3, 2, 8, 10, 1\n\n";
	for (int v : {5, 3, 7, 3, 2, 8, 10, 1}) {
		smm.push(v);
		print_opt("top()", smm.top());
		print_opt("min()", smm.min());
		print_opt("max()", smm.max());
		std::cout << "---\n";
	}

	std::cout << "\nPopping...\n";
	while (!smm.empty()) {
		smm.pop();
		print_opt("top()", smm.top());
		print_opt("min()", smm.min());
		print_opt("max()", smm.max());
		std::cout << "---\n";
	}

	return 0;
}

