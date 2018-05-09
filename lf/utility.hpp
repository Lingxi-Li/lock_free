#ifndef LF_UTILITY_HPP
#define LF_UTILITY_HPP

#include <cstdint>
#include <atomic>

#include "prolog.inc"

inline constexpr auto rlx = std::memory_order_relaxed;
inline constexpr auto rel = std::memory_order_release;
inline constexpr auto acq = std::memory_order_acquire;
inline constexpr auto eat = std::memory_order_consume;
inline constexpr auto cst = std::memory_order_seq_cst;
inline constexpr auto acq_rel = std::memory_order_acq_rel;

template <typename T>
struct counted_ptr {
  static_assert(sizeof(void*) == 8, "Requires 64-bit pointer.");

  T* ptr{};
  std::uint64_t cnt{};
};

#include "epilog.inc"

#endif // LF_UTILITY_HPP
