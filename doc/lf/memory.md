# memory

This header provides utilities for non-array memory management and object (un)initialization.
They are used exclusively in the library implementation.
The built-in utilities are never used directly.

- [Introspective Initialization](#introspective-initialization)
- [Exception Safety](#exception-safety)
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

## Exception Safety

Creating an object in dynamic memory involves two steps, memory allocation and object initialization.
There are two things to note concerning exception safety in this process.

The first thing to note is that if object initialization threw, memory should be deallocated.
Failing to do so results in memory leak. The other thing to note, which is less well-known, is that
initializers should be evaluated after memory allocation. Consider the code.

~~~C++
auto p = std::make_unique<T>( U(std::move(v)) );
~~~

Assume that the initialization process `T( U(std::move(v)) )` does not throw.
Since the initializer expression `U(std::move(v))` is evaluated after memory allocation,
in case memory allocation threw, `v` would have already been moved/modified.
Evaluating initializers after memory allocation avoids this issue.

The (non-placement) new expression does this [[ref][5]].
The code can thus be revised as

~~~C++
std::unique_ptr<T> p(new T(U(std::move(v))));
~~~

Not just that, new expression also makes sure that memory is deallocated if
initialization throws (the first point to note).
But still, the new expression is no panacea.
It does not do [introspective initialization](#introspective-initialization).

This header provides initialization tools that perform introspective initialization
and deallocate memory in case of exception. It also provides helper macros that
combine memory allocation and object initialization, similar to the new expression but
with introspective initialization.

[5]:https://stackoverflow.com/q/49646113/1348273

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
inline constexpr
struct dismiss_t {
  template <typename T>
  void operator()(T* p) const noexcept;
}
dismiss;
~~~

You still invoke with `dismiss(p)` and have auto-deduction in effect.
But you can also pass `dismiss` along like `std::invoke(dismiss, p)`.

This header provides non-template template wrappers.

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
template <typename T>
T* allocate();

template <typename T>
T* try_allocate() noexcept;

inline constexpr
struct deallocate_t {
  void operator()(void* p) const noexcept;
}
deallocate;

inline constexpr
struct init_no_catch_t {
  template <typename T, typename... Us>
  void operator()(T*& p, Us&&... us) const;
  template <typename T, typename... Us>
  void operator()(T* const & p, Us&&... us) const;
}
init_no_catch;

inline constexpr
struct init_t {
  template <typename P, typename... Us>
  void operator()(P&& p, Us&&... us) const;
}
init;

template <typename T, typename... Us>
T emplace(Us&&... us);

LF_MAKE(p, T, ...)

// un-initialize and deallocate
inline constexpr
struct dismiss_t {
  template <typename T>
  void operator()(T* p) const noexcept;
}
dismiss;

template <typename T>
using unique_ptr = std::unique_ptr<T, dismiss_t>;

template <typename T, typename... Us>
auto init_unique(T* p, Us&&... us);

LF_MAKE_UNIQUE(p, T, ...)
~~~

## Details

~~~C++
inline constexpr
struct init_no_catch_t {
  template <typename T, typename... Us>
  void operator()(T*& p, Us&&... us) const;
  template <typename T, typename... Us>
  void operator()(T* const & p, Us&&... us) const;
}
init_no_catch;

inline constexpr
struct init_t {
  template <typename P, typename... Us>
  void operator()(P&& p, Us&&... us) const;
}
init;
~~~

[Intro-initializes](#introspective-initialization) a `T` (value type of `P`) object at the address
referred to by `p` with `us...`, and sets `p` with the [return value of placement new][3] if possible
(i.e., when `p` is a non-const l-value). In case of initialization exception, the `_no_catch` version
does nothing, while the normal version deallocates `p` before propagating the exception.

Note that do not code `init(allocate<T>(), ...)` (and similarly `init_no_catch(allocate<T>(), ...)`).
Since argument evaluation order is unspecified [[ref][6]], the code does not guarantee that
the initializer expression is evaluated after memory allocation, which is crucial to providing
[exception safety](#exception-safety). Code the following instead.

~~~C++
auto p = allocate<T>();
init(p, ...);
~~~

`LF_MAKE(p, T, ...)` is provided to help with this.

[6]:http://en.cppreference.com/w/cpp/language/eval_order

--------------------------------------------------------------------------------

~~~C++
template <typename T, typename... Us>
T emplace(Us&&... us);
~~~

Creates a `T` on call stack with [introspective initialization](#introspective-initialization) from `us...`.
Note that `T` is not required to be copyable nor movable, since [RVO][4] is mandatory since C++17.

[4]:http://en.cppreference.com/w/cpp/language/copy_elision

--------------------------------------------------------------------------------

~~~C++
LF_MAKE(p, T, ...)
~~~

This helper macro combines memory allocation and object initialization.
It guarantees that the initializer expression is evaluated after memory allocation to guard
against memory allocation exception [[ref](#exception-safety)]).
Specifically, it expands to

~~~C++
auto p = allocate<T>();
init(p ...);
~~~

Note that if initializer expression is non-empty, double commas are needed after `T`, e.g., `LF_MAKE(p, T,, a, b);`
If initializer expression is empty, single comma is needed, e.g., `LF_MAKE(p, T,);`

--------------------------------------------------------------------------------

~~~C++
template <typename T>
using unique_ptr = std::unique_ptr<T, dismiss_t>;

template <typename T, typename... Us>
auto init_unique(T* p, Us&&... us);

LF_MAKE_UNIQUE(p, T, ...)
~~~

`init_unique()` is similar to `init()` but additionally returns a `unique_ptr`.
Note that do not code `init_unique(allocate<T>(), ...)`. Since argument evaluation order is unspecified [[ref][6]], the code does not guarantee that the initializer expression is evaluated after memory allocation,
which is crucial to providing [exception safety](#exception-safety). Code the following instead.

~~~C++
auto p = allocate<T>();
init_unique(p, ...);
~~~

`LF_MAKE_UNIQUE(p, T, ...)` expands to

~~~C++
auto some_unique_name = allocate<T>();
auto p = init_unique(some_unique_name ...);
~~~

Note that if initializer expression is non-empty, double commas are needed after `T`,
e.g., `LF_MAKE_UNIQUE(p, T,, a, b);` If initializer expression is empty, single comma is needed,
e.g., `LF_MAKE_UNIQUE(p, T,);`
