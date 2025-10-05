#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <functional>
#include <unordered_map>

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

// Parse a CSV line with columns: id,name,manager_id(optional)
inline bool parse_employee_line(const std::string& line, Employee& out) {
	// Quick reject (no comma)
	if (line.find(',') == std::string::npos) return false;

	std::istringstream iss(line);
	std::string col;

	// id
	if (!std::getline(iss, col, ',')) return false;
	try {
		out.id_ = std::stoi(col);
	} catch (...) { return false; }

	// name
	if (!std::getline(iss, col, ',')) return false;
	out.name_ = col;

	// manager (optional)
	if (std::getline(iss, col, ',')) {
		if (!col.empty()) {
			try { out.manager_id_ = std::stoi(col); }
			catch (...) { out.manager_id_.reset(); }
		} else {
			out.manager_id_.reset();
		}
	} else {
		out.manager_id_.reset();
	}
	return true;
}

class EmployeeDirectory {
	std::unordered_map<int, Employee> employees_;

	public:
	explicit EmployeeDirectory(const std::string& filename) {
		employees_ = load_employees(filename);
	}

	void print() const {
		for (const auto& kv : employees_) std::cout << kv.second;
	}

	void print_tree(int emp_id) const {
		// Build an indent string once; mutate as we traverse
		std::string indent;
		indent.reserve(256);

		std::function<void(int)> dfs = [&](int id) {
			auto it = employees_.find(id);
			if (it == employees_.end()) return;

			std::cout << indent << it->second;

			// Increase indent for children
			indent.append(2, ' ');
			for (int child : it->second.reportees_) {
				dfs(child);
			}
			// Restore indent
			indent.erase(indent.size() >= 2 ? indent.size() - 2 : 0);
		};

		dfs(emp_id);
	}

	private:
	static std::unordered_map<int, Employee> load_employees(const std::string& filename) {
		std::ifstream ifs(filename);
		std::unordered_map<int, Employee> employees;

		if (!ifs) {
			std::cerr << "Failed to open file: " << filename << '\n';
			return employees;
		}

		// Optional: pre-reserve if you can estimate count (e.g., by first counting lines)
		// employees.reserve(1024);

		std::string record;

		// Skip header
		if (!std::getline(ifs, record)) return employees;

		std::vector<std::pair<int,int>> edges;  // (emp_id, manager_id)
		edges.reserve(1024);

		Employee e;
		while (std::getline(ifs, record)) {
			if (record.empty()) continue;
			if (!parse_employee_line(record, e)) continue;

			// Insert or overwrite the employee entry (try_emplace avoids default-construct)
			auto [it, inserted] = employees.try_emplace(e.id_, std::move(e));
			if (!inserted) {
				// If duplicate ID, update name/manager; keep existing reportees_
				it->second.name_ = it->second.name_.empty() ? it->second.name_ : it->second.name_;
			}

			// Save edge if manager present
			if (it->second.manager_id_) {
				edges.emplace_back(it->second.id_, *it->second.manager_id_);
			}
		}

		// Wire reportees in a second pass (O(N))
		for (const auto& [child, mgr] : edges) {
			auto it = employees.find(mgr);
			if (it != employees.end()) it->second.reportees_.push_back(child);
		}

		return employees;
	}
};

int main() {
	EmployeeDirectory empDirectory("employees.csv");
	empDirectory.print();
	empDirectory.print_tree(1);
	return 0;
}
