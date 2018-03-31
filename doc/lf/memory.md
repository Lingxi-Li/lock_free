# memory

This header provides utilities for non-array memory management and object initialization/uninitialization.
They are used exclusively in the library implementation.
The built-in utilities are never used directly.

- [Adaptive Initialization](#adaptive-initialization)
- [Non-Template Wrapper](#non-template-wrapper)
- [Why Not the Built-In Utilities](#why-not-the-built-in-utilities)
- [Synopsis](#synopsis)

## Adaptive Initialization

List initialization using the `{}` syntax is often termed uniform/universal initialization.
It applies to both aggregate and non-aggregate types.
However, consistently using `{}` to perform initialization is problematic in semantics.
For example, `auto p = std::make_unique<std::vector<int>>(2, 1)` makes `[1, 1]` not `[2, 1]`.
Internally, `()` is used instead of `{}` to avoid surprises.
Consequently, the following code does not compile (as of C++17)

~~~C++
struct pr {
  int a, b;
};
auto p = std::make_unique<pr>(1, 2);
~~~

It would be ideal if we could have some kind of adaptive initialization that

1. performs `()` initialization if possible, and
2. falls back to `{}` initialization otherwise, e.g., for aggregate types.

This header provides tools to perform such adaptive initialization.

## Non-Template Wrapper

More often than not, template arguments to function template are auto-deduced.
This is so neat that you may forget its template nature... until you need to pass it along.
For example, the header provides the following function template as a counterpart to the delete expression.

~~~C++
template <typename T>
void dismiss(T* p) noexcept;
~~~

You usually just `dismiss(p)` with `T` auto-deduced from `p`.
But `std::invoke(dismiss, p)` will not work.
In this case, you have to `std::invoke(dismiss<...>, p)` which is annoying.

This issue can be addressed with non-template wrappers.
For example,

~~~C++
const struct deleter_t {
  template <typename T>
  void operator()(T* p) const noexcept {
    dismiss(p);
  }
} deleter;
~~~

`deleter(p)` is equivalent to `dismiss(p)` with auto-deduction in place.
But you can also pass `deleter` along like `std::invoke(deleter, p)`.

This header provides such non-template wrappers.

## Why Not the Built-In Utilities

1. They are untyped and C-style, e.g., see [`operator new()`][2].
   Templates are thus provided with auto-deduced type and size,
   which offer a concise syntax and are less error-prone.
2. Built-in utilities are [delicate][1] and easy to get wrong, leading to intricate bugs.
   An example would be [ignoring the return value of placement new][3].
   Robust counterparts are provided that avoid the pitfalls.
3. The syntax of built-in utilities is somewhat exotic and eye-piercing.
   Counterparts are provided to offer a uniform conventional function call syntax.
4. Using custom allocators is rare in practice.
   Writing allocator-aware data structures may not be worth the trouble.
   Sticking to the standard allocator may be a hassle too,
   for you have to either drag a stateless allocator object along,
   or create a new one every time you need the functionality.
   Besides, the allocator interface is unnecessarily complex based on
   the library's simple needs.
   Free functions are apt here.

[1]:https://stackoverflow.com/q/49546754/1348273
[2]:http://en.cppreference.com/w/cpp/memory/new/operator_new
[3]:https://stackoverflow.com/q/49568858/1348273

## Synopsis

~~~C++
// operator new()
template <typename T>
T* allocate();

// no-throw operator new()
template <typename T>
T* try_allocate() noexcept;

// operator delete()
void deallocate(void* p) noexcept;

// operator delete()
const struct deallocator_t {
  void operator()(void* p) const noexcept;
} deallocator;

// Placement new (adaptive initialization)
template <typename T, typename... Us>
void init(T*& p, Us&&... us);

// new expression (adaptive initialization)
template <typename T, typename... Us>
T* make(Us&&... us);

// delete expression
template <typename T>
void dismiss(T* p) noexcept;

// delete expression (non-template wrapper)
const struct deleter_t {
  template <typename T>
  void operator()(T* p) const noexcept;
} deleter;

template <typename T>
using unique_ptr = std::unique_ptr<T, deleter_t>;

template <typename T, typename... Us>
unique_ptr<T> make_unique(Us&&... us);
~~~
