/* ---------------------------------------------------------
// C++ Keywords (Grouped)
// ---------------------------------------------------------

// Control Flow
if, else, switch, case, default, break, continue, goto, return

// Loops
for, while, do

// Data Types & Modifiers
int, char, float, double, void, bool, wchar_t
short, long, signed, unsigned, auto, decltype

// Storage Class Specifiers
static, extern, mutable, register, thread_local

// Constants & Qualifiers
const, volatile, constexpr

// Type Casts
static_cast, dynamic_cast, reinterpret_cast, const_cast

// Memory Management
new, delete, sizeof, alignof, alignas

// Functions & Operators
inline, noexcept, operator, explicit, friend, virtual, override, final, default, delete

// Classes & Objects
class, struct, union, enum, typename, template, this

// Inheritance & Polymorphism
public, private, protected, virtual

// Exception Handling
try, catch, throw

// Namespace & Scope
namespace, using, ::, typedef

// Logic & Comparison
true, false, nullptr, and, or, not, bitand, bitor, xor

// Other Special Keywords
typeid, static_assert, asm, export, concept, requires, co_await, co_return, co_yield

// Operator Overloads
new, delete, operator

// ===============================
// Keywords: auto and decltype
// ===============================


auto:
- Compiler deduces type from initializer.
- Requires initialization.
- Drops const and reference by default.

decltype:
- Deduces exact type of an expression without evaluating it.
- Preserves const, reference, and rvalue qualifiers.
- Useful for templates, return types, and complex expressions.

decltype(auto):
- Combines both: deduces full type like decltype, with auto syntax.

1. What is the purpose of auto in C++?
→ Type inference: lets the compiler deduce the variable type from the initializer.

2. Can auto deduce reference or const?
→ No. auto drops reference and const unless explicitly specified.

3. What happens when using brace initialization with auto?
→ It creates std::initializer_list<T> if types match, or causes a compile error.

4. How does auto differ from decltype?
→ auto evaluates the expression and may drop qualifiers; decltype deduces exact type without evaluation.

5. Give an example where decltype is better than auto.
→ const int& x = 10;
   auto a = x;        // int
   decltype(x) b = x; // const int&

6. What is decltype((x)) vs decltype(x)?
→ decltype(x) → type of x  
   decltype((x)) → reference type if x is an lvalue

7. What is decltype(auto)?
→ It deduces type like decltype, but with auto syntax. Preserves const/ref/rvalue qualifiers.

8. Where are auto and decltype commonly used?
→ auto: loops, lambdas, iterators, return types  
   decltype: templates, return type deduction, exact type inspection


📝 Tip: Use `auto` to simplify long type names; use `decltype` when type precision matters.

// ===============================
// Keyword: static
// ===============================

The `static` keyword changes the linkage, lifetime, or visibility of a variable or function depending on the context.

// ✅ Use Cases
// - Inside function: retains value between calls.
// - Inside class: shared across all objects.
// - Global scope: limits linkage to file.
// - Static member function: no access to `this`.


1. Purpose inside a function? → Retains state.
2. Static class members? → Shared across instances.
3. Can static function access non-static data? → No.
4. Effect at file scope? → Makes symbol private to file.
5. Why use in singleton/logger? → Persistent, shared instance.


// ===============================
// Keyword: mutable
// ===============================

Allows modification of a class member even from const member functions or const objects.

// - Applies to non-static members only.
// - Useful for caches, counters, flags.


1. What does `mutable` allow? → Modify in const context.
2. On static members? → No.
3. Real use case? → Caches, debug counters.
4. Const object modifies mutable? → Yes.
5. In multithreading? → Common with mutex.


// ===============================
// Keyword: const
// ===============================

Marks variables, functions, and pointers as immutable or read-only.

// - `const int a` → value cannot change.
// - `const int* p` vs `int* const p` → value vs pointer constness.
// - `const` on member functions → ensures no modification.
// - `const` object → can only call const member functions.

1. Difference: `const int*` vs `int* const`? → Value vs pointer constness.
2. What is a const member function? → Cannot modify data.
3. Override const with non-const? → No.
4. Why use with references? → Prevent accidental changes.
5. Can constructors be const? → No.
6. Related keywords? → const_cast, mutable, constexpr.



// ===============================
// Keyword: constexpr
// ===============================

Allows values/functions to be evaluated at compile time.

// - Variables must be initialized with constant expressions.
// - Functions: single return (C++11), relaxed in C++14+.
// - Useful in templates, array bounds, switch cases.

1. const vs constexpr? → const = immutability, constexpr = compile-time.
2. Runtime call to constexpr? → Yes, if used in runtime context.
3. Rules in C++11? → Single return, no loops/if.
4. Constructors? → Yes, since C++11.
5. Templates? → Useful for compile-time logic.
6. Non-const args? → Treated as regular function.


// ===============================
// Keywords: extern, volatile, register, thread_local

extern       → Variable/function defined elsewhere.
volatile     → Prevents optimization; value may change outside program.
register     → Hints storage in CPU register (deprecated).
thread_local → Each thread gets its own copy.  ( thread_local int id = 0 )

1. extern vs static linkage? → extern = global, static = file-local.
2. volatile in loops? → Prevents optimization; reloads every time.
3. Is volatile thread-safe? → No.
4. Why can't you take &register? → May not exist in memory.
5. Why deprecated? → Compilers optimize better.
6. Use of thread_local? → Per-thread storage.
7. Can thread_local be global? → Yes.
8. thread_local vs static? → static is shared, thread_local is per-thread.



// ===============================
// Keywords: inline, noexcept, operator, explicit, friend,
//           virtual, override, final, default, delete
// ===============================



inline     → Suggests inlining and avoids linker errors in headers.
noexcept   → Declares no exceptions will be thrown.
operator   → Enables custom operator overloads.
explicit   → Disables implicit conversions.
friend     → Grants access to private/protected members.
virtual    → Enables dynamic polymorphism.
override   → Ensures function overrides base class virtual.
final      → Prevents overriding or inheritance.
default    → Generates default implementation.
delete     → Disables specific function usage.


1. Why use inline? → Inlining and header-safe definitions.
2. noexcept? → Optimization and contract for no exceptions.
3. operator? → Custom behavior for +, ==, etc.
4. explicit? → Avoid unwanted conversions.
5. friend? → Access private members.
6. virtual? → Enable runtime dispatch.
7. override? → Safer overriding.
8. final? → Lock down inheritance.
9. = default? → Compiler-generated code.
10. = delete? → Prevent copy, assign, etc.
11. virtual + override + final? → All valid together.


// ===============================
// Keywords: class, struct, union, enum, typename, template, this
// ===============================

class      → Defines user-defined type (default private).
struct     → Like class, but default public.
union      → Shares memory among members.
enum       → Defines named integral constants.
typename   → Marks type placeholder in templates.
template   → Enables generic code.
this       → Pointer to current object.

1. class vs struct? → Only access specifier differs.
2. When union? → Save memory; one member at a time.
3. enum vs enum class? → enum = unscoped; enum class = scoped, type-safe.
4. typename usage? → Tells compiler a name is a type.
5. typename vs class? → Interchangeable in templates.
6. this pointer? → Refers to current object.
7. Why templates? → Generic, reusable code.
8. Class inside template? → Yes, supported.


// ===============================
// Keywords: public, private, protected, virtual
// ===============================

public     → Members accessible everywhere.
private    → Members accessible only within the class.
protected  → Members accessible in class and derived classes.
virtual    → Enables runtime polymorphism and dynamic dispatch.

 public vs private vs protected? → Controls access level to members.
2. When is a virtual function needed? → For dynamic (runtime) dispatch.
3. Why should a base class destructor be virtual? → To ensure correct destruction via base pointer.
4. Can constructors be virtual? → No, constructors can't be virtual.
5. What is slicing? → Loss of derived parts when assigned to base object.



// ===============================
// Keywords: try, catch, throw
// ===============================


try     → Block that may throw exceptions.
catch   → Handles exceptions.
throw   → Raises an exception.


1. No matching catch? → std::terminate is called.
2. Can throw custom types? → Yes, including user-defined classes.
3. Catch block order? → First matching catch is executed.
4. What is exception safety? → Guarantees around consistent program state.
5. Can destructors throw? → They should not; may trigger std::terminate.



// ===============================
// Keywords: namespace, using, ::, typedef
// ===============================


namespace  → Groups identifiers into a named scope.
using      → Brings names from a namespace into current scope.
::         → Scope resolution operator to access global or nested scope.
typedef    → Creates a type alias (modern C++ prefers `using`).


1. Why use namespaces? → To avoid name collisions in large projects.
2. What does 'using namespace std;' do? → Imports all names from std into global scope.
3. Purpose of '::' operator? → To qualify names from global or enclosing scopes.
4. typedef vs using? → using is more flexible and preferred in modern C++.
5. Are nested namespaces allowed? → Yes, and they can be aliased.


// ===============================
// Keyword: nullptr
// ===============================


nullptr → A type-safe null pointer literal introduced in C++11.


1. NULL vs nullptr? → nullptr = std::nullptr_t, avoids overload ambiguity.
2. Why introduced? → Safer null representation.
3. Compatible with any pointer? → Yes.
4. Overload for nullptr? → Yes, using std::nullptr_t.
5. Type of nullptr? → std::nullptr_t.



// ===============================
// Keywords: new, delete, sizeof, alignof, alignas
// ===============================


new       → Allocates and constructs.
delete    → Destructs and deallocates.
sizeof    → Gets type/object size at compile time.
alignof   → Gets alignment requirement of a type.
alignas   → Specifies required alignment.


1. malloc/free vs new/delete? → new/delete run constructors.
2. sizeof on heap object? → Gives pointer size.
3. alignof? → Alignment in bytes.
4. alignas? → Set specific alignment.
5. delete invalid pointer? → Undefined behavior.
6. Override new/delete? → Yes.
7. Default alignments? → Platform dependent.
8. Does new return aligned memory? → Yes.

*/

int main()
{
	return 0;
}
