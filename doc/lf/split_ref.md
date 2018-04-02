# split_ref

This header provides utilities for implementing the split reference counts scheme.

- [Split Reference Counts](#split-reference-counts)
  - [Encoding](#encoding)
- [Synopsis](#synopsis)
- [Details](#details)

## Split Reference Counts

This is an extension of the [ordinary reference counting scheme][3].
It distinguishes and counts external and internal references.
External references are transient.
Internal references are ordinary persistent references.

### Encoding

Use a single 64-bit unsigned integer to encode both external and internal count.
The higher/lower 32-bits encode external/internal count, respectively.

*Notes:* Use unsigned type to wrap around rather than overflowing [[ref][1]].
Use higher bits for external count to allow external count wrap around without
interfering with internal count.
External count wrap around is OK, but not internal count.

[3]: https://en.wikipedia.org/wiki/Reference_counting
[1]: http://en.cppreference.com/w/cpp/language/operator_arithmetic#Overflows

## Synopsis

~~~C++
// One external count.
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
template <typename T, typename Del = dismiss_t>
void unhold_ptr_acq(T* p, Del&& del = Del{}) noexcept;

// Unholds the pointer by committing/releasing multiple references.
template <typename T, typename Del = dismiss_t>
void unhold_ptr_acq(
 counted_ptr<T> cp,
 std::uint64_t int_cnt,
 Del&& del = Del{}) noexcept;

// Unholds the pointer by committing/releasing multiple references.
template <typename T, typename Del = dismiss_t>
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

`atomic_counted_ptr` has a trivial default constructor that does nothing [[ref][2]].
This effectively ignores `counted_ptr`'s default member initializers [[ref][6]].
To zero-initialize an `atomic_counted_ptr`, use value initialization [[ref][7]].

`atomic_counted_ptr` requires 64-bit pointers to work, which make
`counted_ptr` 128-bit with no padding. This avoids the [atomic padding issue][4].
Support for lock-free 128-bit atomic operations is pervasive among modern CPUs [[ref][5]].

[4]: https://stackoverflow.com/q/48947428/1348273
[5]: https://superuser.com/a/941175/517080
[2]: http://en.cppreference.com/w/cpp/atomic/atomic/atomic
[6]: https://stackoverflow.com/q/49387069/1348273
[7]: https://stackoverflow.com/q/49400942/1348273

--------------------------------------------------------------------------------

~~~C++
template <typename T>
void hold_ptr(
 atomic_counted_ptr<T>& stub,
 counted_ptr<T>& ori,
 std::memory_order mem_ord) noexcept;

template <typename T>
bool hold_ptr_if_not_null(
 atomic_counted_ptr<T>& stub,
 counted_ptr<T>& ori,
 std::memory_order mem_ord) noexcept;
~~~

Holds `stub` with an external reference.
`ori` is passed an initial guess of `stub`.
The resulting value is stored back to `ori`.
`mem_ord` specifies the memory order semantics of the read-modify-write operation on `stub`.

`hold_ptr_if_not_null()` fails if `stub.ptr` is found to be null.
In this case, it returns false and does not modify `stub`, ignoring `mem_ord`.
If `ori.ptr` is passed null, fails immediately.

--------------------------------------------------------------------------------

~~~C++
template <typename T, typename Del = dismiss_t>
void unhold_ptr_acq(T* p, Del&& del = Del{}) noexcept;
~~~

Unholds `p` by releasing one external reference.
If no reference then remains, deletes `p` using the supplied deleter `del`,
and `unhold_ptr_rel()`, if any, happens-before the deletion.
`T` is required to have member `std::atomic_uint64_t cnt`
with the split reference counts [encoding](#encoding).

--------------------------------------------------------------------------------

~~~C++
template <typename T, typename Del = dismiss_t>
void unhold_ptr_acq(
 counted_ptr<T> cp,
 std::uint64_t int_cnt,
 Del&& del = Del{}) noexcept;

template <typename T, typename Del = dismiss_t>
void unhold_ptr_rel(
 counted_ptr<T> cp,
 std::uint64_t int_cnt,
 Del&& del = Del{}) noexcept;
~~~

Unholds `p` by

1. releasing one external reference,
2. committing the external count `cp.cnt`,
3. releasing `int_cnt` internal references.

If no reference then remains, deletes `cp.ptr` using the supplied deleter `del`.
`unhold_ptr_rel()`, if any, happens-before the deletion performed by `unhold_ptr_acq()`.
`T` is required to have member `std::atomic_uint64_t cnt`
with the split reference counts [encoding](#encoding).
