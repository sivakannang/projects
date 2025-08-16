
/*
==========================
C++ Templates — Notes
==========================
- Templates are compile-time blueprints (functions, classes, aliases, variables, NTTPs).
- Key mechanics: deduction → instantiation → overload resolution; SFINAE prunes invalid candidates.
- Forwarding refs: T&& where T is deduced; preserve value category via std::forward<T>(x).
- Concepts (C++20) > SFINAE for readability/diagnostics; use requires/constraints to control overload sets.
- Variadic templates + fold expressions (C++17) remove recursion boilerplate; use (expr op ...) forms.
- CTAD (C++17) deduces class template args from constructor; add deduction guides when needed.
- Partial specialization: only for class/alias/variable templates; functions use overloading/constraints instead.
- Template template parameters let you pass containers/policies (e.g., std::vector) as parameters.
- Dependent names: prefix with 'typename' when referring to a type; use 'template' keyword for dependent template calls.
- Performance: prefer exact types & inlining; watch ODR bloat from many instantiations; explicitly instantiate where helpful.

==========================
Interview Questions (with crisp answers)
==========================
1) SFINAE—what/why? → Substitution Failure Is Not An Error; removes invalid template candidates during substitution to enable conditional overloads.
2) Forwarding ref vs rvalue ref? → T&& with deduced T is a forwarding ref; T&& with non-deduced T (e.g., function param of concrete type) is an rvalue ref.
3) Perfect forwarding recipe? → template <class T> f(T&& x){ g(std::forward<T>(x)); }.
4) When prefer concepts over SFINAE? → Whenever on C++20+; clearer constraints, better errors, simpler code.
5) NTTPs—what can they be? → Integrals, enums, pointers/refs with linkage, certain literal/structural types (C++20 expands this).
6) CTAD gotcha? → Deduction may pick unintended types; add explicit deduction guides or specify <T> explicitly.
7) Why 'typename' sometimes? → T::value_type is dependent; 'typename' tells the compiler it’s a type, not a static member.
8) Why 'template' sometimes? → For dependent template members: obj.template foo<U>().
9) Partial vs full specialization? → Full matches exact arguments; partial matches a pattern (e.g., T*). Functions don’t allow partial spec—use overloads.
10) Fold expressions—why useful? → Simpler, safer expansions over parameter packs vs recursive templates.
11) Greedy variadic ctor problem? → It can steal overloads; constrain with concepts/SFINAE or use tag types.
12) Templates & unused branches? → Ill-formed code in non-instantiated branches is OK; only instantiated paths are checked (plus immediate context rules).

*/

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <functional>
#include <concepts>

#define PRINT(sec) std::cout << "\n--- " << sec << " ---\n"

// ------------------------------------------------------------
// 1) SFINAE vs Concepts
// ------------------------------------------------------------

// SFINAE: only participates if T is integral
template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T twice_sfinae(T x) { return x + x; }

// Concepts: cleaner constraint
template <std::integral T>
T twice_concept(T x) { return x + x; }

// ------------------------------------------------------------
// 2) Perfect forwarding wrapper
// ------------------------------------------------------------
template <class F, class... Args>
decltype(auto) call_forward(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

struct Tracer {
    Tracer() = default;
    Tracer(const Tracer&)         { std::cout << "[copy] "; }
    Tracer(Tracer&&) noexcept     { std::cout << "[move] "; }
};

void take(const Tracer&) { std::cout << "L\n"; }
void take(Tracer&&)      { std::cout << "R\n"; }

// ------------------------------------------------------------
// 3) Class Template + CTAD (Class Template Argument Deduction)
// ------------------------------------------------------------
template <class T>
struct Named {
    T value;
    std::string name;
    Named(T v, std::string n) : value(std::move(v)), name(std::move(n)) {}
};
// (CTAD deduces T from ctor args)

// ------------------------------------------------------------
// 4) Variadic templates + fold expressions
// ------------------------------------------------------------
template <class... Ts>
auto sum(Ts... xs) { return (xs + ...); } // ((x1 + x2) + ...)

// Ordered printing with left-to-right side effects
template <class... Ts>
void print_line(Ts&&... xs) {
    ((std::cout << std::forward<Ts>(xs)), ...);
    std::cout << '\n';
}

// ------------------------------------------------------------
// 5) Template template parameter
// ------------------------------------------------------------
template <class T, template<class...> class Cont>
struct Uses {
    Cont<T> c;
};

// ------------------------------------------------------------
// 6) Partial specialization (traits-style)
// ------------------------------------------------------------
template <class T>
struct Traits { static constexpr bool fast = false; };

template <>
struct Traits<int> { static constexpr bool fast = true; };

// ------------------------------------------------------------
// 7) Templated lambda (C++20)
// ------------------------------------------------------------
auto scaler = []<class T>(T v, T s) { return v * s; };

// ------------------------------------------------------------
// 8) Dependent name example (typename keyword)
// ------------------------------------------------------------
template <class C>
void use_value_type(typename C::value_type v) {
    std::cout << "value_type passed = " << v << '\n';
}

// ------------------------------------------------------------
int main() {
    // 1) SFINAE vs Concepts
    PRINT("SFINAE vs Concepts");
    std::cout << "twice_sfinae(7)   = " << twice_sfinae(7)   << '\n';
    std::cout << "twice_concept(12) = " << twice_concept(12) << '\n';
    // The following would fail to compile (as intended constraints):
    // std::cout << twice_sfinae(3.14) << '\n';
    // std::cout << twice_concept(3.14) << '\n';

    // 2) Perfect forwarding demo (disambiguate overloaded 'take')
    PRINT("Perfect forwarding");
    auto take_l = static_cast<void(*)(const Tracer&)>(&take);
    auto take_r = static_cast<void(*)(Tracer&&)>(&take);

    Tracer t;
    std::cout << "call_forward(take_l, t)        -> ";
    call_forward(take_l, t);            // L

    std::cout << "call_forward(take_r, Tracer{}) -> ";
    call_forward(take_r, Tracer{});     // [move] R

    // 3) CTAD with class template
    PRINT("CTAD (Class Template Argument Deduction)");
    Named n1{42, std::string{"answer"}};     // Named<int>
    Named n2{std::string{"Siva"}, "name"};   // Named<std::string>
    std::cout << "n1: [" << n1.value << ", " << n1.name << "]\n";
    std::cout << "n2: [" << n2.value << ", " << n2.name << "]\n";

    // 4) Variadics / folds
    PRINT("Variadics & folds");
    std::cout << "sum(1,2,3,4) = " << sum(1,2,3,4) << '\n';
    print_line("Hello ", "variadic ", "world ", 2025, "!");

    // 5) Template template parameter
    PRINT("Template template parameter");
    Uses<int, std::vector> u;
    u.c.push_back(10);
    u.c.push_back(20);
    std::cout << "Uses<int, vector>.size = " << u.c.size() << '\n';

    // 6) Partial specialization check
    PRINT("Partial specialization (Traits)");
    static_assert(Traits<int>::fast, "int should be fast");
    static_assert(!Traits<std::string>::fast, "string should not be fast");
    std::cout << "Traits<int>::fast = " << Traits<int>::fast
              << ", Traits<std::string>::fast = " << Traits<std::string>::fast << '\n';

    // 7) Templated lambda
    PRINT("Templated lambda");
    std::cout << "scaler(3, 5)       = " << scaler(3, 5) << '\n';         // int
    std::cout << "scaler(2.5, 0.4)   = " << scaler(2.5, 0.4) << '\n';     // double

    // 8) Dependent name usage
    PRINT("Dependent name (typename)");
    use_value_type<std::vector<int>>(123);

    std::cout << "\nAll sections ran.\n";
    return 0;
}

