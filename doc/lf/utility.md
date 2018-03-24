# utility
This header provides low-level utilities for building lock-free data structures.

- [Split Reference Counts](#split-reference-counts)
  - [Encoding](#encoding)
- [Synopsis](#synopsis)
- [Details](#details)

## Split Reference Counts
This is an extension of the [ordinary reference counting scheme][refcnt].
It distinguishes and counts external and internal references.
External references are transient.
Internal references are ordinary persistent references.

### Encoding
Use a single 64-bit unsigned integer to encode both external and internal count.
The higher/lower 32-bits encode external/internal count, respectively.

*Notes:* Use unsigned type to wrap around rather than overflowing [[ref][of]].
Use higher bits for external count to allow external count wrap around without
interfering with internal count.
External count wrap around is OK, but not internal count.

[refcnt]: https://en.wikipedia.org/wiki/Reference_counting
[of]: http://en.cppreference.com/w/cpp/language/operator_arithmetic#Overflows

## Synopsis
~~~C++
// Shorthand for memory order semantics
constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto eat = std::memory_order_consume;
constexpr auto cst = std::memory_order_seq_cst;
constexpr auto acq_rel = std::memory_order_acq_rel;

// The encoding for one external count.
constexpr auto ext_cnt = (std::uint64_t)1 << 32;

// Counted pointer structure.
template <typename T>
struct counted_ptr {
  T* ptr{};
  std::uint64_t cnt{};
};

// Atomic counted pointer.
template <typename T>
using atomic_counted_ptr = std::atomic<counted_ptr<T>>;

// Holds the pointer with an external reference.
template <typename T>
void hold_ptr(
 atomic_counted_ptr<T>& stub,
 counted_ptr<T>& ori,
 std::memory_order mem_ord) noexcept;

// Holds the pointer with an external reference if it is not null.
template <typename T>
bool hold_ptr_if_not_null(
 atomic_counted_ptr<T>& stub,
 counted_ptr<T>& ori,
 std::memory_order mem_ord) noexcept;

// Unholds the pointer by releasing an external reference.
template <typename T, typename Del = std::default_delete<T>>
void unhold_ptr_acq(T* p, Del&& del = Del{}) noexcept;

// Unholds the pointer by committing/releasing multiple references.
template <typename T, typename Del = std::default_delete<T>>
void unhold_ptr_acq(
 counted_ptr<T> cp,
 std::uint64_t int_cnt,
 Del&& del = Del{}) noexcept;

// Unholds the pointer by committing/releasing multiple references.
template <typename T, typename Del = std::default_delete<T>>
void unhold_ptr_rel(
 counted_ptr<T> cp,
 std::uint64_t int_cnt,
 Del&& del = Del{}) noexcept;
~~~

## Details

~~~C++
template <typename T>
struct counted_ptr {
  T* ptr{};
  std::uint64_t cnt{};
};

template <typename T>
using atomic_counted_ptr = std::atomic<counted_ptr<T>>;
~~~
There are many uses of `cnt`.
In the [split reference counts scheme](#split-reference-counts),
it is the uncommitted external count.
Before dereferencing `ptr`, hold the pointer with an external reference
by increasing `cnt` by `ext_cnt`.

`atomic_counted_ptr` has a trivial default constructor that does nothing [[ref][actor]].
This effectively ignores `counted_ptr`'s default member initializers [[ref][trivial]].
To zero-initialize an `atomic_counted_ptr`, use value initialization [[ref][valinit]].

`atomic_counted_ptr` requires 64-bit pointers to work, which make
`counted_ptr` 128-bit with no padding. This avoids the [atomic padding issue][atompad].
Support for lock-free 128-bit atomic operations is pervasive among modern CPUs [[ref][16b]].

[atompad]: https://stackoverflow.com/q/48947428/1348273
[16b]: https://superuser.com/a/941175/517080
[actor]: http://en.cppreference.com/w/cpp/atomic/atomic/atomic
[trivial]: https://stackoverflow.com/q/49387069/1348273
[valinit]: https://stackoverflow.com/q/49400942/1348273
