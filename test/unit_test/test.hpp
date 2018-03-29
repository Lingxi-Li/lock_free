#define CATCH_CONFIG_DISABLE_MATCHERS
#include "catch.hpp"

#include <cstdint>
#include <cstring>
#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

#define STRINGIFY_(...) #__VA_ARGS__
#define STRINGIFY(...) STRINGIFY_(__VA_ARGS__)

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define REQUIRE_SAME_T(...) static_assert( \
  std::is_same<__VA_ARGS__>{}, \
  #__VA_ARGS__ " are not the same.")

#define AT_EXIT(...) \
  auto CONCAT(at_exit_f_, __LINE__) = [&] { __VA_ARGS__; }; \
  ::impl::at_exit<decltype(CONCAT(at_exit_f_, __LINE__))> \
    CONCAT(at_exit_, __LINE__){ ::std::move(CONCAT(at_exit_f_, __LINE__)) }

namespace impl {

template <typename F>
struct at_exit {
  F f;
  ~at_exit() {
    f();
  }
};

} // namespace impl

namespace {

template <typename Cnt = std::atomic_uint64_t>
struct counted {
  template <typename T>
  counted(T c) noexcept:
   valid(true), cnt(c) {
    ++inst_cnt;
  }

 ~counted() {
    if (valid) --inst_cnt;
  }

  counted(const counted& ci) noexcept:
   valid(ci.valid), cnt(ci.cnt) {
    if (valid) ++inst_cnt;
  }

  counted(counted&& ci) noexcept:
   valid(ci.valid), cnt(ci.cnt) {
    ci.valid = false;
  }

  counted& operator=(counted ci) noexcept {
    swap(*this, ci);
    return *this;
  }

  friend void swap(counted& a, counted& b) noexcept {
    std::swap(a.valid, b.valid);
    std::swap(a.cnt, b.cnt);
  }

  bool valid;
  Cnt cnt;

  static int inst_cnt;
};

template <typename Cnt>
int counted<Cnt>::inst_cnt = 0;

} // unnamed namespace

template <typename F, typename T>
void for_each(F&& f, T&& v) {
  std::forward<F>(f)(std::forward<T>(v));
}

template <typename F, typename T, typename... Us>
void for_each(F&& f, T&& v, Us&&... us) {
  std::forward<F>(f)(std::forward<T>(v));
  for_each(std::forward<F>(f), std::forward<Us>(us)...);
}

template <typename T>
T* alloc(int fill = -1) {
  auto p = (T*)operator new(sizeof(T));
  std::memset(p, fill, sizeof(T));
  return p;
}

template <typename T>
bool memcmp(const T& a, const T& b) noexcept {
  return std::memcmp(&a, &b, sizeof(a)) == 0;
}
