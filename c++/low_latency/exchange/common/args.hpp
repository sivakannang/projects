#pragma once
// Tiny command-line parser: only --key=value and --flag.
//
// Options are declared where they are read (get / flag). done() then rejects anything
// that was never declared, so a typo like --rat=50000 fails loudly instead of being
// silently ignored (a silently ignored setting ruins a benchmark without you noticing).
//
//   lab::Args args(argc, argv, "Validates and matches orders");
//   int cpu = args.get<int>("cpu", -1, "CPU to pin to (-1 = don't pin)");
//   args.done();   // handles --help, unknown options, bad values

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace lab {

class Args {
public:
    Args(int argc, char** argv, std::string description) : description_(std::move(description)) {
        std::string_view prog = argc > 0 ? argv[0] : "prog";
        prog_ = std::string(prog.substr(prog.find_last_of('/') + 1));

        for (int i = 1; i < argc; ++i) {
            std::string_view a = argv[i];
            if (a.size() <= 2 || a.substr(0, 2) != "--") {
                errors_.push_back("unexpected argument '" + std::string(a) + "' (options look like --key=value)");
                continue;
            }
            a.remove_prefix(2);
            size_t eq = a.find('=');
            std::string key(a.substr(0, eq));
            if (eq == std::string_view::npos) {
                bare_.insert(key);
                values_[key] = "";
            } else {
                bare_.erase(key);
                values_[key] = std::string(a.substr(eq + 1));  // repeated option: last one wins
            }
        }
    }

    // Value option, e.g. --rate=50000. Returns def when absent.
    template <typename T>
    T get(const std::string& key, T def, const char* help) {
        describe("--" + key + "=<value>", help, toString(def));
        auto it = values_.find(key);
        if (it == values_.end()) return def;
        if (bare_.count(key)) {
            errors_.push_back("--" + key + " needs a value, e.g. --" + key + "=" + toString(def));
            return def;
        }
        T value{};
        if (!parse(it->second, value)) {
            errors_.push_back("bad value for --" + key + ": '" + it->second + "'");
            return def;
        }
        return value;
    }

    // On/off option, e.g. --verbose
    bool flag(const std::string& key, const char* help) {
        describe("--" + key, help, "");
        if (!values_.count(key)) return false;
        if (!bare_.count(key)) errors_.push_back("--" + key + " takes no value");
        return true;
    }

    // Call after all get/flag calls: prints --help, or exits on any error
    void done() {
        if (values_.count("help")) {
            std::printf("%s: %s\n\nOptions:\n", prog_.c_str(), description_.c_str());
            for (const std::string& line : help_) std::printf("%s\n", line.c_str());
            std::printf("  %-24s show this help\n", "--help");
            std::exit(0);
        }
        for (const auto& kv : values_)
            if (!known_.count(kv.first)) errors_.push_back("unknown option --" + kv.first);
        if (errors_.empty()) return;
        for (const std::string& e : errors_) std::fprintf(stderr, "%s: %s\n", prog_.c_str(), e.c_str());
        std::fprintf(stderr, "run '%s --help' to see the options\n", prog_.c_str());
        std::exit(2);
    }

private:
    void describe(const std::string& usage, const char* help, const std::string& def) {
        known_.insert(usage.substr(2, usage.find('=') - 2));
        std::string line = "  " + usage;
        line.resize(std::max<size_t>(line.size() + 1, 27), ' ');
        line += help;
        if (!def.empty()) line += " [default: " + def + "]";
        help_.push_back(line);
    }

    static bool parse(const std::string& s, std::string& out) {
        out = s;
        return true;
    }

    template <typename T>
    static bool parse(const std::string& s, T& out) {
        if constexpr (std::is_integral_v<T>) {
            auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
            return ec == std::errc{} && end == s.data() + s.size();
        } else {
            static_assert(std::is_floating_point_v<T>, "Args::get supports integers, floating point and std::string");
            char* end = nullptr;
            out = T(std::strtod(s.c_str(), &end));
            return !s.empty() && end == s.c_str() + s.size();
        }
    }

    template <typename T>
    static std::string toString(const T& v) {
        if constexpr (std::is_same_v<T, std::string>) {
            return v;
        } else if constexpr (std::is_integral_v<T>) {
            return std::to_string(v);
        } else {
            char buf[32];
            std::snprintf(buf, sizeof buf, "%g", double(v));
            return buf;
        }
    }

    std::string prog_;
    std::string description_;
    std::map<std::string, std::string> values_;
    std::set<std::string> bare_;   // given as --key without '='
    std::set<std::string> known_;  // declared via get / flag
    std::vector<std::string> help_;
    std::vector<std::string> errors_;
};

}  // namespace lab
