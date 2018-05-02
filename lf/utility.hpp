#ifndef LF_UTILITY_HPP
#define LF_UTILITY_HPP

#include <cassert>
#include <cstddef>
#include <atomic>
#include <iterator>
#include <stdexcept>

#include "prolog.inc"

#define LF_IMPL_C(a, ...) a##__VA_ARGS__

#define LF_C(a, ...) LF_IMPL_C(a, __VA_ARGS__)
#define LF_UNI_NAME(prefix) LF_C(prefix, LF_C(LF_C(_at_line_, __LINE__), _))

inline constexpr auto rlx = std::memory_order_relaxed;
inline constexpr auto rel = std::memory_order_release;
inline constexpr auto acq = std::memory_order_acquire;
inline constexpr auto eat = std::memory_order_consume;
inline constexpr auto cst = std::memory_order_seq_cst;
inline constexpr auto acq_rel = std::memory_order_acq_rel;

template <typename InIt>
std::size_t range_extent(InIt first, InIt last) {
  auto n = std::distance(first, last);
  assert(n >= 0);
  return n >= 0 ? n :
         throw std::invalid_argument("Invalid range.");
}

template <typename T>
void plain_store(std::atomic<T>& atom, T val) noexcept {
  new(&atom) std::atomic<T>(val);
}

#include "epilog.inc"

#endif // LF_UTILITY_HPP
