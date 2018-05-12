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

inline constexpr auto null = std::uint32_t(-1);

struct cp_t {
  std::uint32_t ptr{null};
  std::uint32_t cnt{};
};

static_assert(std::atomic<cp_t>::is_always_lock_free);

#include "epilog.inc"

#endif // LF_UTILITY_HPP
