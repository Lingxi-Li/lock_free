## Header `lf/stack.hpp`

This header provides a general stack data structure.

- [Synopsis](#synopsis)
- [Details](#details)

### Synopsis

~~~C++
template <typename T>
class stack {
  static_assert(std::is_move_constructible_v<T>);

public:
  stack() noexcept;

  template <typename BiIt>
  stack(BiIt first, BiIt last);

  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  template <typename... Us>
  void emplace(Us&&... us);

  template <typename BiIt>
  void bulk_push(BiIt first, BiIt last);

  std::optional<T> try_pop() noexcept;
};
~~~

### Details

~~~C++
template <typename BiIt>
stack(BiIt first, BiIt last);
~~~

`BiIt` models [BidirectionalIterator][1].
Pushes elements in the range [`first`, `last`) in turn.
The last-pushed element is the new stack top.
Provides strong exception safety guarantee.

--------------------------------------------------------------------------------

~~~C++
template <typename... Us>
void emplace(Us&&... us);
~~~

Pushes a new element [intro-initialized](memory.md#introspective-initialization) from `us...`.
Strong exception safe if the initialization is strong exception safe.

--------------------------------------------------------------------------------

~~~C++
template <typename BiIt>
void bulk_push(BiIt first, BiIt last);
~~~

`BiIt` models [BidirectionalIterator][1].
Pushes elements in the range [`first`, `last`) in turn.
The last-pushed element is the new stack top.
Provides strong exception safety guarantee.

[1]:http://en.cppreference.com/w/cpp/concept/BidirectionalIterator
