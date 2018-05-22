#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cctype>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#define ERROR(...) throw ::std::runtime_error(::mkstr(__VA_ARGS__))
#define CONFIRM_MEMORY_FOOTPRINT(...) ::impl::confirm_memory_footprint(#__VA_ARGS__, __VA_ARGS__)

using tick = std::chrono::high_resolution_clock;

template <typename T>
using matrix = std::vector<std::vector<T>>;

class sync_point {
public:
  void wait() noexcept {
    wait_cnt.fetch_add(1, std::memory_order_release);
    while (wait_cnt.load(std::memory_order_acquire) != 0);
  }

  void wait_and_signal(unsigned expect_wait_cnt) noexcept {
    while (wait_cnt.load(std::memory_order_acquire) < expect_wait_cnt);
    wait_cnt.store(0, std::memory_order_release);
  }

private:
  std::atomic_uint wait_cnt{0};
};

inline constexpr auto operator""_K(unsigned long long v) noexcept { return v * 1000; }
inline constexpr auto operator""_M(unsigned long long v) noexcept { return v * 1000'000; }
inline constexpr auto operator""_B(unsigned long long v) noexcept { return v * 1000'000'000; }

template <typename F, typename InputIt>
void for_each_element(F&& f, InputIt first, InputIt last) {
  while (first != last) {
    std::invoke(std::forward<F>(f), *first++);
  }
}

template <typename... Us>
std::string mkstr(const Us&... vs) {
  std::ostringstream os;
  (void)(os << ... << vs);
  return os.str();
}

inline void validate_thread_cnt(std::size_t thread_cnt) {
  if (thread_cnt == 0) ERROR("thread_cnt == 0");
  if (thread_cnt > std::thread::hardware_concurrency()) {
    ERROR("thread_cnt > hardware_concurrency (",
          std::thread::hardware_concurrency(), ')');
  }
}

inline void prompt() {
  std::cout << "Continue? [y/n] " << std::flush;
  std::string str;
  std::getline(std::cin, str);
  if (str != "y") ERROR("User cancel.");
}

namespace impl {

inline
void print_memory_footprint(std::size_t tot, const char*) {
  std::cout << "  -------------\n"
            << "  total: " << tot / 1_M << "M\n";
}

template <typename... Args>
void print_memory_footprint(std::size_t tot, const char* p, std::size_t sz, Args... args) {
  std::cout << "  " << p << ": " << sz / 1_M << "M\n";
  print_memory_footprint(tot + sz, std::strchr(p, '\0') + 1, args...);
}

template <typename... Args>
void confirm_memory_footprint(const char* str, Args... args) {
  std::string names;
  while (auto c = *str++) {
    if (std::isspace(c)) continue;
    names.push_back(c == ',' ? '\0' : c);
  }
  std::cout << "Estimated memory footprint\n";
  print_memory_footprint(0, names.c_str(), args...);
  prompt();
}

} // namespace impl

#endif // UTILITY_HPP
