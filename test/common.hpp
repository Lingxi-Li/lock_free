#ifndef LF_TEST_COMMON_HPP
#define LF_TEST_COMMON_HPP

#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

#define LF_TEST_BEGIN(name) void name##_test() {
#define LF_TEST_END std::cout << __func__ << "\t\tpassed" << std::endl; }
#define LF_TEST_RUN(name) name##_test();

#define LF_TEST_SAME_T(a, b) static_assert(::std::is_same<a, b>{}, "");

template <typename T, typename Func, typename... Args>
bool throw_e(Func&& func, Args&&... args) {
  bool caught = false;
  try {
    std::forward<Func>(func)(
      std::forward<Args>(args)...);
  }
  catch (const T&) {
    caught = true;
  }
  return caught;
}

#endif // LF_TEST_COMMON_HPP
