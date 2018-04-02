# utility

This header provides miscellaneous utilities.

- [Synopsis](#synopsis)
- [Details](#details)

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

// Generic dispatching tags.
struct dispatch_0_tag;
struct dispatch_1_tag : dispatch_0_tag;
struct dispatch_2_tag : dispatch_1_tag;
~~~

## Details

~~~C++
template <typename InIt>
std::size_t range_extent(InIt first, InIt last);
~~~

If `first`, `last` do not specify a valid range,
throws `std::invalid_argument`.
Otherwise, returns size of the range.

--------------------------------------------------------------------------------

~~~C++
struct dispatch_0_tag;
struct dispatch_1_tag : dispatch_0_tag;
struct dispatch_2_tag : dispatch_1_tag;
~~~

Generic tags used for [tag dispatching][1].
The higher the number, the more specialized the tag is.
E.g., `dispatch_0_tag` is most generic and is able to bind to any tag.

[1]:https://www.boost.org/community/generic_programming.html#tag_dispatching
