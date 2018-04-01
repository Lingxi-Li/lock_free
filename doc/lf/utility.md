# utility

This header provides general utilities.

## Synopsis

~~~C++
// Shorthand of memory order semantics
constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto eat = std::memory_order_consume;
constexpr auto cst = std::memory_order_seq_cst;
constexpr auto acq_rel = std::memory_order_acq_rel;

// Checks range validity and gets its size.
template <typename InIt>
std::size_t range_extent(InIt first, InIt last);
~~~
