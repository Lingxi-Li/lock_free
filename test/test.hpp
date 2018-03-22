#define CATCH_CONFIG_DISABLE_MATCHERS
#include "catch.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#define REQUIRE_SAME_T(...) static_assert( \
  std::is_same<__VA_ARGS__>{}, \
  #__VA_ARGS__ " are not the same.")

namespace {

struct counted {
  counted(std::uint64_t c = 0) noexcept:
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
    if (valid) ci.valid = false;
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
  std::uint64_t cnt;

  static int inst_cnt;
};

int counted::inst_cnt = 0;

} // anonymous namespace

template <typename T>
auto alloc(int fill = -1) {
  auto deleter = [](T* p) noexcept {
      p->~T();
      operator delete(p);
  };
  auto p = (T*)operator new(sizeof(T));
  std::memset(p, fill, sizeof(T));
  return std::unique_ptr<T, decltype(deleter)>(p, deleter);
}

template <typename T>
bool memcmp(const T& a, const T& b) noexcept {
  return std::memcmp(&a, &b, sizeof(a)) == 0;
}
