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
    REQUIRE(memcmp(p->tri, triple{1, 2, 3}));
    REQUIRE(p->vec.size() == 2);
    REQUIRE(p->vec[0] == 1);
    REQUIRE(p->vec[1] == 1);
    p->ci.require(7, 1);
    lf::uninit(p);
    REQUIRE(ci_t::inst_cnt == 0);
    lf::deallocate(p);
  }
}
