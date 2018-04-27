#define CATCH_CONFIG_DISABLE_MATCHERS
#include "catch.hpp"

#include <cstdint>
#include <cstring>
#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#define REQUIRE_SAME_T(...) static_assert( \
  std::is_same<__VA_ARGS__>{}, \
  #__VA_ARGS__ " are not the same.")

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

  inline static int inst_cnt = 0;
};

} // unnamed namespace

struct triple {
  int a, b, c;
};

template <typename F>
void for_each(F&&) noexcept {}

template <typename F, typename T, typename... Us>
void for_each(F&& f, T&& v, Us&&... us) {
  std::invoke(std::forward<F>(f), std::forward<T>(v));
  for_each(std::forward<F>(f), std::forward<Us>(us)...);
}

template <typename T>
void memset(T& v, int fill = -1) noexcept {
  std::memset((void*)&v, fill, sizeof(T));
}

template <typename T>
T* alloc(int fill = -1) {
  auto p = (T*)operator new(sizeof(T));
  memset(*p, fill);
  return p;
}

template <typename T>
bool memcmp(const T& a, const T& b) noexcept {
  return std::memcmp(&a, &b, sizeof(a)) == 0;
}
