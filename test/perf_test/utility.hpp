#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <atomic>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#define ERROR(...) throw ::std::runtime_error(::mkstr(__VA_ARGS__))

using tick = std::chrono::high_resolution_clock;

template <typename T>
using matrix = std::vector<std::vector<T>>;

class sync_point {
public:
  void wait() const noexcept { while (!cont.load(std::memory_order_relaxed)); }
  void signal() noexcept { cont.store(true, std::memory_order_relaxed); }

private:
  std::atomic_bool cont{false};
};

inline constexpr int operator""_K(unsigned long long v) noexcept { return (int)v * 1000; }
inline constexpr int operator""_M(unsigned long long v) noexcept { return (int)v * 1000'000; }
inline constexpr int operator""_B(unsigned long long v) noexcept { return (int)v * 1000'000'000; }

template <typename... Us>
std::string mkstr(const Us&... vs) {
  std::ostringstream os;
  (void)(os << ... << vs);
  return os.str();
}

inline void validate_thread_cnt(unsigned thread_cnt) {
  if (thread_cnt == 0) ERROR("thread_cnt == 0");
  if (thread_cnt > std::thread::hardware_concurrency()) {
    ERROR("thread_cnt > hardware_concurrency (",
          std::thread::hardware_concurrency(), ')');
  }
}

#endif // UTILITY_HPP_