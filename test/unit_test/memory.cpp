#include "../../lf/memory.hpp"
#include "../../lf/memory.hpp"

#include "test.hpp"

#include <cstdint>
#include <atomic>
#include <vector>

using ci_t = counted<int>;
using veci_t = std::vector<int>;

namespace {

void require_1_1(veci_t* p) {
  REQUIRE(p->size() == 2);
  REQUIRE(p->at(0) == 1);
  REQUIRE(p->at(1) == 1);
}

void require_2_1(veci_t* p) {
  REQUIRE(p->size() == 2);
  REQUIRE(p->at(0) == 2);
  REQUIRE(p->at(1) == 1);
}

} // unnamed namespace

TEST_CASE("memory") {

  SECTION("memory") {
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

  SECTION("init") {
    auto p0 = lf::allocate<veci_t>();
    auto p1 = lf::allocate<veci_t>();
    auto p2 = alloc<std::atomic_int>();
    REQUIRE(*p2 == -1);

    SECTION("init") {
      lf::init(p0);
      lf::init(p1, 2, 1);
      lf::init(p2);
      REQUIRE(p0->empty());
      require_1_1(p1);
      REQUIRE(*p2 == 0);
    }

    SECTION("list_init") {
      lf::list_init(p0);
      lf::list_init(p1, 2, 1);
      lf::list_init(p2);
      REQUIRE(p0->empty());
      require_2_1(p1);
      REQUIRE(*p2 == 0);
    }

    for_each(dismiss, p0, p1, p2);
  }

  SECTION("make") {
    auto p0 = lf::make<veci_t>();
    auto p1 = lf::make<veci_t>(2, 1);
    auto p2 = lf::make<std::atomic_int>();
    auto p3 = lf::list_make<veci_t>();
    auto p4 = lf::list_make<veci_t>(2, 1);
    auto p5 = lf::list_make<std::atomic_int>();

    REQUIRE(p0->empty());
    require_1_1(p1);
    REQUIRE(*p2 == 0);
    REQUIRE(p3->empty());
    require_2_1(p4);
    REQUIRE(*p5 == 0);

    for_each(dismiss, p0, p1, p2, p3, p4, p5);
  }

  SECTION("dismiss") {
    auto p0 = lf::make<ci_t>(1);
    auto p1 = lf::make<ci_t>(2);
    REQUIRE(ci_t::inst_cnt == 2);
    lf::dismiss(p0);
    REQUIRE(ci_t::inst_cnt == 1);
    lf::deleter{}(p1);
    REQUIRE(ci_t::inst_cnt == 0);
  }

}
