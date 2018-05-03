#define NDEBUG

#include "../../lf/utility.hpp"
#include "../../lf/utility.hpp"

#include "test.hpp"

#include <atomic>
#include <iterator>
#include <list>
#include <vector>

namespace {

template <typename C>
void test_range_extent(const C& c) {
  REQUIRE(std::size(c) > 0);
  REQUIRE(lf::range_extent(std::begin(c), std::end(c)) == std::size(c));
  REQUIRE(lf::range_extent(std::begin(c), std::begin(c)) == 0);
}

} // unnamed namespace

TEST_CASE("utility") {

  SECTION("memory order shorthand") {
    REQUIRE_SAME_T(decltype(lf::rlx), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::rel), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::eat), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::cst), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq_rel), const std::memory_order);
    static_assert(lf::rlx == std::memory_order_relaxed);
    static_assert(lf::rel == std::memory_order_release);
    static_assert(lf::acq == std::memory_order_acquire);
    static_assert(lf::eat == std::memory_order_consume);
    static_assert(lf::cst == std::memory_order_seq_cst);
    static_assert(lf::acq_rel == std::memory_order_acq_rel);
  }

  SECTION("range_extent") {
    std::vector<int> vec(5);
    std::list<int> lis(5);
    test_range_extent(vec);
    REQUIRE_THROWS_AS(
      lf::range_extent(vec.end(), vec.begin()),
      std::invalid_argument);
    test_range_extent(lis);
  }

}
