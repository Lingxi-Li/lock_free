#ifndef LF_TEST_COMMON_HPP
#define LF_TEST_COMMON_HPP

#include <cassert>
#include <iostream>

#define LF_TEST_BEGIN(name) void name##_test() {
#define LF_TEST_END std::cout << __func__ << " passed" << std::endl; }
#define LF_TEST_RUN(name) name##_test();

#endif // LF_TEST_COMMON_HPP
