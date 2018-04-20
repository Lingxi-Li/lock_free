#include "cli.hpp"

#include <atomic>
#include <chrono>

inline auto tick() {
  return std::chrono::high_resolution_clock::now()
    .time_since_epoch().count();
}

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
