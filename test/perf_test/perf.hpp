#include "cli.hpp"

#include <atomic>

inline constexpr int operator""_K(unsigned long long v) noexcept { return (int)v * 1000; }
inline constexpr int operator""_M(unsigned long long v) noexcept { return (int)v * 1000'000; }
inline constexpr int operator""_B(unsigned long long v) noexcept { return (int)v * 1000'000'000; }

class sync_point {
public:
  void wait() const noexcept {
    while (!cont.load(std::memory_order_relaxed));
  }

  void go() noexcept {
    cont.store(true, std::memory_order_relaxed);
  }

private:
  std::atomic_bool cont{false};
};
