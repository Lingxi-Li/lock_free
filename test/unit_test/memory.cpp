#include "../../lf/memory.hpp"
#include "../../lf/memory.hpp"

#include "test.hpp"

#include <initializer_list>
#include <vector>

using ci_t = counted<int>;
using veci_t = std::vector<int>;

namespace {

struct list_init {
  explicit list_init(std::initializer_list<int>) noexcept {
    // nop
  }
};

struct stru {
  triple tri;
  veci_t vec;
  list_init lst;
  ci_t ci;
};

void require(const triple& tri, const veci_t& vec, const ci_t& ci) {
  REQUIRE(memcmp(tri, triple{1, 2, 3}));
  REQUIRE(vec.size() == 2);
  REQUIRE(vec[0] == 1);
  REQUIRE(vec[1] == 1);
  ci.require(7, 1);
}

} // unnamed namespace

TEST_CASE("memory") {
  SECTION("allocate/deallocate") {
    REQUIRE_FALSE(lf::allocate<int>(0));
    auto p = lf::allocate<int>();
    REQUIRE(p);
    p[0] = 1;
    auto pp = lf::allocate<int>(2);
    REQUIRE(pp);
    p[0] = 1;
    p[1] = 2;
    lf::deallocate(pp);
    lf::deallocate(p);
    lf::deallocate(nullptr);
  }
  SECTION("init/uninit") {
    auto p = alloc<stru>();
    lf::init(&p->tri, 1, 2, 3);
    lf::init(&p->vec, 2, 1);
    lf::init(&p->lst, 1, 2, 3);
    lf::init(&p->ci, 7);
    require(p->tri, p->vec, p->ci);
    lf::uninit(p);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p);
    auto ptri = alloc<triple>();
    auto pvec = alloc<veci_t>();
    auto plis = alloc<list_init>();
    auto pci  = alloc<ci_t>();
    lf::init(ptri, 1, 2, 3);
    lf::init(pvec, 2, 1);
    lf::init(plis, 1, 2, 3);
    lf::init(pci, 7);
    require(*ptri, *pvec, *pci);
    lf::uninit(ptri);
    lf::uninit(pvec);
    lf::uninit(plis);
    lf::uninit(pci);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(ptri);
    lf::deallocate(pvec);
    lf::deallocate(plis);
    lf::deallocate(pci);
  }
}
