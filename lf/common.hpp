#ifndef LF_COMMON_HPP
#define LF_COMMON_HPP

#include <cstdint>
#include <atomic>

namespace lf {

// Assumes that transient ref count occupies the higher 32 bits.
// Persistent ref count occupies the lower 32 bits.
// Both work as unsigned type to wrap-around and avoid overflow.
// Combine into a single 64-bit integer, so that the two can be
// updated together in an atomic fasion.
constexpr auto one_trefcnt = (std::uint64_t)1 << 32;

// trefcnt: Transient ref count.
// To release a counted_ptr, add trefcnt to the combined count,
// and then deal with persistent ref count if necessary.
// atomic<counted_ptr> should almost always be defined as mutable.
// Also note https://stackoverflow.com/q/48947428/1348273
template <typename T>
struct counted_ptr {
  T* p;
  std::uint64_t trefcnt;
};

template <typename T>
using atomic_counted_ptr = std::atomic<counted_ptr<T>>;

} // namespace lf

#endif // LF_COMMON_HPP
