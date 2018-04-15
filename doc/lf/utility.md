## Header `lf/utility.hpp`

This header provides miscellaneous utilities.

- [Synopsis](#synopsis)
- [Details](#details)

## Synopsis

~~~C++
// Expands arguments and concatenates
LF_C(a, ...)

// Expands to a prefixed line # based name.
LF_UNI_NAME(prefix)

// Shorthand of memory order semantics
inline constexpr auto rlx = std::memory_order_relaxed;
inline constexpr auto rel = std::memory_order_release;
inline constexpr auto acq = std::memory_order_acquire;
inline constexpr auto eat = std::memory_order_consume;
inline constexpr auto cst = std::memory_order_seq_cst;
inline constexpr auto acq_rel = std::memory_order_acq_rel;

// Checks range validity and gets its size.
template <typename InIt>
std::size_t range_extent(InIt first, InIt last);
~~~

## Details

~~~C++
template <typename InIt>
std::size_t range_extent(InIt first, InIt last);
~~~

If `first`, `last` do not specify a valid range,
throws `std::invalid_argument`.
Otherwise, returns size of the range.
