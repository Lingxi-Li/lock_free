#include "../../lf/memory.hpp"
#include "../../lf/memory.hpp"

#include "test.hpp"

#include <cstdint>
#include <atomic>
#include <vector>

using ci_t = counted<int>;
using veci_t = std::vector<int>;
using upci_t = lf::unique_ptr<ci_t>;

namespace {

void require(veci_t* pvec, triple* ptri, std::atomic_int* patm) {
  REQUIRE(pvec->size() == 2);
  REQUIRE(pvec->at(0) == 1);
  REQUIRE(pvec->at(1) == 1);
  REQUIRE(memcmp(*ptri, triple{1, 2, 3}));
  REQUIRE(*patm == 0);
}

} // unnamed namespace

TEST_CASE("memory") {

  SECTION("memory") {
    REQUIRE(ci_t::inst_cnt == 0);
    auto p0 = lf::allocate<ci_t>();
    REQUIRE(p0 != nullptr);
    REQUIRE(ci_t::inst_cnt == 0);
    auto p1 = lf::try_allocate<ci_t>();
    REQUIRE(p1 != nullptr);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p0);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p1);
    REQUIRE(ci_t::inst_cnt == 0);
  }

  SECTION("init") {
    auto pvec = alloc<veci_t>();
    auto ptri = alloc<triple>();
    auto patm = alloc<std::atomic_int>();

    lf::init(pvec, 2, 1);
    lf::init(ptri, 1, 2, 3);
    lf::init(patm);
    require(pvec, ptri, patm);

    for_each(lf::dismiss, pvec, ptri, patm);
  }

  SECTION("make") {
    auto pvec = lf::make<veci_t>(2, 1);
    auto ptri = lf::make<triple>(1, 2, 3);
    auto patm = lf::make<std::atomic_int>();
    require(pvec, ptri, patm);
    for_each(lf::dismiss, pvec, ptri, patm);
  }

  SECTION("dismiss") {
    auto p0 = lf::make<ci_t>(0), p1 = lf::make<ci_t>(0);
    REQUIRE(ci_t::inst_cnt == 2);
    lf::dismiss(p0);
    REQUIRE(ci_t::inst_cnt == 1);
    lf::dismiss(p1);
    REQUIRE(ci_t::inst_cnt == 0);
  }

  SECTION("unique_ptr") {
    REQUIRE_SAME_T(upci_t::deleter_type, lf::dismiss_t);
    REQUIRE(ci_t::inst_cnt == 0);
    {
      auto up = lf::make_unique<ci_t>(1);
      REQUIRE_SAME_T(decltype(up), upci_t);
      REQUIRE(up);
      REQUIRE(up->cnt == 1);
      REQUIRE(ci_t::inst_cnt == 1);
    }
    REQUIRE(ci_t::inst_cnt == 0);
  }

}
