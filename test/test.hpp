#define CATCH_CONFIG_DISABLE_MATCHERS
#include "catch.hpp"

#include <type_traits>

#define REQUIRE_SAME_T(...) static_assert( \
    std::is_same<__VA_ARGS__>{}, \
    #__VA_ARGS__ " are not the same.")
