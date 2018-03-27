#ifndef LF_UTILITY_HPP
#define LF_UTILITY_HPP

#include <cassert>
#include <cstddef>
#include <atomic>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace lf {

constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto eat = std::memory_order_consume;
constexpr auto cst = std::memory_order_seq_cst;
constexpr auto acq_rel = std::memory_order_acq_rel;

template <typename T>
T* allocate() {
  return (T*)operator new(sizeof(T));
}

template <typename T>
T* try_allocate() noexcept {
  return (T*)operator new(sizeof(T), std::nothrow);
}

inline
void deallocate(void* p) noexcept {
  operator delete(p);
}

template <typename FwdIt>
std::size_t range_extent(FwdIt first, FwdIt last) {
  auto n = std::distance(first, last);
  assert(n >= 0);
  return n >= 0 ? n :
         throw std::invalid_argument("Invalid range.");
}

template <typename T, typename... Us>
void construct(T* p, Us&&... us) {
  new(p) T(std::forward<Us>(us)...);
}

} // namespace lf

#endif // LF_UTILITY_HPP
