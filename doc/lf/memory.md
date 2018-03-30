# memory

This header provides utilities for memory management and object initialization/uninitialization.

- [Why Not the Built-In Utilities](#why-not-the-built-in-utilities)
- [Synopsis](#synopsis)

## Why Not the Built-In Utilities

1. They are untyped and C-style, e.g., see [`operator new()`][2].
   Templates are thus provided with auto-deduced type and size,
   which offer a concise syntax and are less error-prone.
2. Built-in utilities are [delicate][1] and easy to get wrong, leading to intricate bugs.
   An example would be [ignoring the return value of placement new][3].
   Robust counterparts are provided that avoid the pitfalls.
3. The syntax of built-in utilities is somewhat exotic and eye-piercing.
   Counterparts are provided to offer a uniform conventional function call syntax.

[1]:https://stackoverflow.com/q/49546754/1348273
[2]:http://en.cppreference.com/w/cpp/memory/new/operator_new
[3]:https://stackoverflow.com/q/49568858/1348273

## Synopsis

~~~C++
// Typed operator new().
template <typename T>
T* allocate();

// Typed no-throw operator new().
template <typename T>
T* try_allocate() noexcept;

// operator delete(p).
void deallocate(void* p) noexcept;

// p = new(p) T(...) with deduced T.
template <typename T, typename... Us>
void init(T*& p, Us&&... us);

// p = new(p) T{...} with deduced T.
template <typename T, typename... Us>
void list_init(T*& p, Us&&... us);

// new T(...) counterpart.
template <typename T, typename... Us>
T* make(Us&&... us);

// new T{...} counterpart.
template <typename T, typename... Us>
T* list_make(Us&&... us);

// delete p counterpart.
template <typename T>
void dismiss(T* p) noexcept;

// std::default_delete counterpart.
template <typename T>
struct deleter {
  void operator()(T* p) const noexcept;
};
~~~
