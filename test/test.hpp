#define CATCH_CONFIG_DISABLE_MATCHERS
#include "catch.hpp"

#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#define REQUIRE_SAME_T(...) static_assert( \
  std::is_same<__VA_ARGS__>{}, \
  #__VA_ARGS__ " are not the same.")

namespace {

struct counted_int {
  counted_int(int v = 0) noexcept:
   val(v) {
    ++cnt;
  }

 ~counted_int() {
    if (valid) --cnt;
  }

  counted_int(const counted_int& ci) noexcept:
   counted_int(ci.val) {
  }

  counted_int(counted_int&& ci) noexcept:
   val(ci.val) {
    ci.valid = false;
  }

  counted_int& operator=(counted_int ci) noexcept {
    std::swap(*this, ci);
    return *this;
  }

  bool valid{true};
  int val;

  static int cnt;
};

int counted_int::cnt = 0;

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
