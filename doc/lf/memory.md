# memory

This header provides utilities for memory management and object initialization/uninitialization.

- [Quirk of the non-array new/delete expression](#quirk)
- [Synopsis](#synopsis)

<a name="quirk"/>

## Quirk of the non-array new/delete expression

To put in a nutshell, effects of the new expression are

1. allocate memory,
2. initialize the object at the allocated memory.

While effects of the delete expression are

1. uninitialize the object at the allocated memory,
2. deallocate memory.

The quirk is that

1. delete expression can only be used on pointers returned by a new expression,
2. pointers returned by a new expression can only be deleted by a delete expression.

Sometimes, memory allocation and initialization cannot be done in a single step.
You have to manually allocate the memory, do something else, and then initialize the object,
e.g., to provide strong exception guarantee.
In this case, the delete expression cannot be used on the resulting pointer [[ref][1]].
Consequently, you have to manually uninitialize and deallocate, which is tedious.
To make things worse, in case both new expression and the manual method are employed,
you have to track which one is used for each object.

This header provides new/delete expression counterparts that do not have this quirk.

[1]:https://stackoverflow.com/q/49546754/1348273

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

// new(p) T(...) with deduced T.
template <typename T, typename... Us>
void init(T* p, Us&&... us);

// new(p) T{...} with deduced T.
template <typename T, typename... Us>
void list_init(T* p, Us&&... us);

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
