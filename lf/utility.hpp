#ifndef LF_UTILITY_HPP
#define LF_UTILITY_HPP

#include <cassert>
#include <cstddef>
#include <atomic>
#include <iterator>
#include <stdexcept>

namespace lf {

constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto eat = std::memory_order_consume;
constexpr auto cst = std::memory_order_seq_cst;
constexpr auto acq_rel = std::memory_order_acq_rel;

template <typename InIt>
std::size_t range_extent(InIt first, InIt last) {
  auto n = std::distance(first, last);
  assert(n >= 0);
  return n >= 0 ? n :
         throw std::invalid_argument("Invalid range.");
}

} // namespace lf

#endif // LF_UTILITY_HPP
