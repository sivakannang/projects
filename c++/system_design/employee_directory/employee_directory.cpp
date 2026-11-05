/*
   g++ -std=c++17 -O2 -Wall -Wextra employee_directory.cpp -o employee_directory
   ./employee_directory      ( run it from this folder, it reads employees.csv )

   Loads employees and their managers from a CSV, and prints the reporting tree.

   Two passes, so a manager can appear in the file after their reportees :
     - First read all the employees into a map, O(N) on average
     - Then link each employee to their manager

   Data from a file can't be trusted. A bad line is reported and skipped, and a manager cycle can't hang or crash the print.
   print_tree is recursive, so a very deep tree would need an explicit stack.
*/

#include <charconv>
#include <functional>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct Employee {
	int id_{-1};
	std::string name_;
	std::optional<int> manager_id_;
	std::vector<int> reportees_;

	friend std::ostream& operator<<(std::ostream& os, const Employee& emp) {
		os << "id: " << emp.id_
			<< " , name: " << emp.name_
			<< " , manager_id: " << (emp.manager_id_ ? *emp.manager_id_ : -1)
			<< " , reportees : ";
		for (int e : emp.reportees_) os << e << ' ';
		return os << '\n';
	}
};

// The whole text must be a number. std::stoi would read "12abc" as 12.
inline std::optional<int> parse_int(const std::string& text) {
	int value = 0;
	const char* end = text.data() + text.size();
	auto [ptr, ec] = std::from_chars(text.data(), end, value);
	if (ec != std::errc{} || ptr != end) return std::nullopt;
	return value;
}

// Parse a CSV line with columns: id,name,manager_id(optional)
inline std::optional<Employee> parse_employee_line(const std::string& line) {
	std::istringstream iss(line);
	std::string col;
	Employee out;

	// id
	if (!std::getline(iss, col, ',')) return std::nullopt;
	auto id = parse_int(col);
	if (!id) return std::nullopt;
	out.id_ = *id;

	// name
	if (!std::getline(iss, col, ',') || col.empty()) return std::nullopt;
	out.name_ = col;

	// manager (optional)
	if (std::getline(iss, col, ',') && !col.empty()) {
		out.manager_id_ = parse_int(col);
		if (!out.manager_id_) return std::nullopt;
	}
	return out;
}

class EmployeeDirectory {
	std::unordered_map<int, Employee> employees_;

	public:
	// Throws std::runtime_error if the file can't be opened
	explicit EmployeeDirectory(const std::string& filename) {
		employees_ = load_employees(filename);
	}

	// The order is not sorted, as it comes from an unordered_map
	void print() const {
		for (const auto& kv : employees_) std::cout << kv.second;
	}

	void print_tree(int emp_id) const {
		// Build an indent string once; mutate as we traverse
		std::string indent;
		indent.reserve(256);

		// Without this, a manager cycle in the data recurses until the stack overflows
		std::unordered_set<int> visited;

		std::function<void(int)> dfs = [&](int id) {
			auto it = employees_.find(id);
			if (it == employees_.end()) return;

			if (!visited.insert(id).second) {
				std::cout << indent << "id: " << id << " already printed, the data has a cycle\n";
				return;
			}

			std::cout << indent << it->second;

			// Increase indent for children
			indent.append(2, ' ');
			for (int child : it->second.reportees_) {
				dfs(child);
			}
			// Restore indent
			indent.resize(indent.size() - 2);
		};

		dfs(emp_id);
	}

	private:
	static std::unordered_map<int, Employee> load_employees(const std::string& filename) {
		std::ifstream ifs(filename);
		if (!ifs) throw std::runtime_error("Failed to open file: " + filename);

		std::unordered_map<int, Employee> employees;
		std::string record;

		// Skip header
		if (!std::getline(ifs, record)) return employees;

		std::vector<int> file_order;     // The map has no order, and reportees should stay in file order

		int line_no = 1;
		while (std::getline(ifs, record)) {
			++line_no;
			if (record.empty()) continue;

			auto employee = parse_employee_line(record);
			if (!employee) {
				std::cerr << "Line " << line_no << " skipped, invalid : " << record << '\n';
				continue;
			}

			int id = employee->id_;
			if (employees.try_emplace(id, std::move(*employee)).second) {
				file_order.push_back(id);
			} else {
				std::cerr << "Line " << line_no << " skipped, duplicate id : " << id << '\n';
			}
		}

		// Wire reportees in a second pass (O(N)). Each employee is in the map once, so each gets one edge.
		for (int id : file_order) {
			const Employee& employee = employees.at(id);
			if (!employee.manager_id_) continue;

			auto manager = employees.find(*employee.manager_id_);
			if (manager == employees.end()) {
				std::cerr << "Employee " << id << " has an unknown manager : " << *employee.manager_id_ << '\n';
				continue;
			}
			manager->second.reportees_.push_back(id);
		}

		return employees;
	}
};

int main() {
	try {
		EmployeeDirectory empDirectory("employees.csv");
		empDirectory.print();
		empDirectory.print_tree(1);
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}
