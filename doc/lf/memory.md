# memory

This header provides utilities for memory management and object construction/destruction.

## Synopsis

~~~C++
// Typed operator new().
template <typename T>
T* allocate();

// Typed no-throw operator new().
template <typename T>
T* try_allocate() noexcept;

// Alias of operator delete().
void deallocate(void* p) noexcept;

// Constructs T(std::forward<Us>(us)...) at p.
template <typename T, typename... Us>
void construct(T* p, Us&&... us);
~~~
