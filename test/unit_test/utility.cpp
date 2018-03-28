#define NDEBUG

#include "../../lf/utility.hpp"
#include "../../lf/utility.hpp"

#include "test.hpp"

#include <atomic>
#include <list>
#include <memory>
#include <vector>

namespace {

template <typename C>
void test_range_extent(const C& c) {
  REQUIRE(c.size() > 0);
  REQUIRE(lf::range_extent(c.begin(), c.end()) == c.size());
  REQUIRE(lf::range_extent(c.begin(), c.begin()) == 0);
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
    static_assert(lf::rlx == std::memory_order_relaxed, "");
    static_assert(lf::rel == std::memory_order_release, "");
    static_assert(lf::acq == std::memory_order_acquire, "");
    static_assert(lf::eat == std::memory_order_consume, "");
    static_assert(lf::cst == std::memory_order_seq_cst, "");
    static_assert(lf::acq_rel == std::memory_order_acq_rel, "");
  }

  SECTION("memory") {
    using ci_t = counted<int>;
    REQUIRE(ci_t::inst_cnt == 0);
    auto p0 = lf::allocate<ci_t>();
    REQUIRE_SAME_T(decltype(p0), ci_t*);
    REQUIRE(p0 != nullptr);
    REQUIRE(ci_t::inst_cnt == 0);
    auto p1 = lf::try_allocate<ci_t>();
    REQUIRE_SAME_T(decltype(p1), ci_t*);
    REQUIRE(p1 != nullptr);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p0);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p1);
    REQUIRE(ci_t::inst_cnt == 0);
  }

  using veci_t = std::vector<int>;

  SECTION("range_extent") {
    veci_t vec(5);
    test_range_extent(vec);
    REQUIRE_THROWS_AS(
      lf::range_extent(vec.end(), vec.begin()),
      std::invalid_argument);
    std::list<int> lis(5);
    test_range_extent(lis);
  }

  SECTION("construct") {
    SECTION("basic") {
      auto p = lf::allocate<veci_t>();
      lf::construct(p, 2, 1);
      std::unique_ptr<veci_t> up(p);
      REQUIRE(p->size() == 2);
      REQUIRE(p->at(0) == 1);
      REQUIRE(p->at(1) == 1);
    }

    SECTION("aggregate value init") {
      struct stru {
        int v;
      };
      auto up = alloc<stru>();
      REQUIRE(up->v == -1);
      lf::construct(up.get());
      REQUIRE(up->v == 0);
    }

    SECTION("atomic value init") {
      auto up = alloc<std::atomic_uint64_t>();
      REQUIRE(*up == std::uint64_t(-1));
      lf::construct(up.get());
      REQUIRE(*up == 0);
    }
  }

}
