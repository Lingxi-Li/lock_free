# memory

This header provides utilities for non-array memory management and object (un)initialization.
They are used exclusively in the library implementation.
The built-in utilities are never used directly.

- [Introspective Initialization](#introspective-initialization)
- [Non-Template Template Wrapper](#non-template-template-wrapper)
- [Why Not Built-In Utilities](#why-not-built-in-utilities)
- [Synopsis](#synopsis)
- [Details](#details)

## Introspective Initialization

List initialization using the `{}` syntax is often termed uniform/universal initialization.
It applies to both aggregate and non-aggregate types.
However, consistently doing list initialization is problematic in semantics.
Because of this, `auto p = std::make_unique<std::vector<int>>(2, 1)` makes `[1, 1]` not `[2, 1]`.
To avoid surprises, `()` is used instead of `{}` internally.
The following code therefore does not compile (as of C++17)

~~~C++
struct pr {
  int a, b;
};
auto p = std::make_unique<pr>(1, 2);
~~~

It would be ideal if there is some kind of introspective initialization that

1. does `()` initialization if possible, and
2. falls back to `{}` initialization otherwise, e.g., for aggregate types.

This header provides tools to perform such introspective initialization.

## Non-Template Template Wrapper

More often than not, template arguments to function template are auto-deduced.
This is so neat that you may forget its template nature... until you need to pass it along.
Suppose the following function template

~~~C++
template <typename T>
void dismiss(T* p) noexcept;
~~~

It is common practice to invoke the function template with `dismiss(p)`, and have `T` auto-deduced from `p`.
However, this does not work for `std::invoke(dismiss, p)`.
In this case, one may have to `std::invoke(dismiss<...>, p)`, which is annoying.

This issue can be addressed with a non-template template wrapper.

~~~C++
const struct dismiss_t {
  template <typename T>
  void operator()(T* p) const noexcept;
} dismiss;
~~~

You still invoke with `dismiss(p)` and have auto-deduction in effect.
But you can also pass `dismiss` along like `std::invoke(dismiss, p)`.

This header provides non-template template wrappers when appropriate.

## Why Not Built-In Utilities

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
   Besides, the allocator interface is unnecessarily complex, considering
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
const struct deallocate_t {
  void operator()(void* p) const noexcept;
} deallocate;

// placement new (introspective initialization)
template <typename P, typename... Us>
void init(P&& p, Us&&... us);

// new expression (introspective initialization)
template <typename T, typename... Us>
T* make(Us&&... us);

// delete expression (non-template template wrapper)
const struct dismiss_t {
  template <typename T>
  void operator()(T* p) const noexcept;
} dismiss;

template <typename T>
using unique_ptr = std::unique_ptr<T, dismiss_t>;

template <typename T, typename... Us>
unique_ptr<T> make_unique(Us&&... us);
~~~

## Details

~~~C++
template <typename P, typename... Us>
void init(P&& p, Us&&... us);
~~~

This ultimate initialization function template initializes the object
at the address referred to by raw pointer `p` with `us...`.
It is similar to `new(p) ...`, but is more powerful in that

- It performs [introspective initialization](#introspective-initialization).
- It is robust by always setting `p` with the [return value of placement new][3]
  if possible (i.e., when `p` is a non-const l-value).
