#include "../../lf/stack.hpp"
#include "../../lf/stack.hpp"

#include "test.hpp"

using ci_t = counted<int>;

namespace {

void require_capacity_2(lf::stack<ci_t>& stk) {
  REQUIRE_FALSE(stk.try_pop());
  REQUIRE(stk.try_push(ci_t(1)));
  REQUIRE(stk.try_push(ci_t(2)));
  REQUIRE_FALSE(stk.try_push(ci_t(3)));
  REQUIRE(ci_t::inst_cnt == 2);
  REQUIRE(stk.try_pop().value().cnt == 2);
  REQUIRE(stk.try_pop().value().cnt == 1);
  REQUIRE_FALSE(stk.try_pop());
  REQUIRE(ci_t::inst_cnt == 0);
}

} // unnamed namespace

TEST_CASE("stack") {
  SECTION("ctor/dtor") {
    lf::stack<ci_t> s1, s2(0), s3(2);
    REQUIRE(ci_t::inst_cnt == 0);
    REQUIRE_FALSE(s1.try_pop());
    REQUIRE_FALSE(s2.try_pop());
    require_capacity_2(s3);
    {
      lf::stack<ci_t> s(2);
      REQUIRE(s.try_push(ci_t(1)));
      REQUIRE(s.try_push(ci_t(2)));
      REQUIRE(ci_t::inst_cnt == 2);
    }
    REQUIRE(ci_t::inst_cnt == 0);
  }
  SECTION("reset") {
    lf::stack<ci_t> s;
    REQUIRE_FALSE(s.try_push(ci_t(1)));
    s.reset(2);
    require_capacity_2(s);
    REQUIRE(s.try_push(ci_t(1)));
    REQUIRE(s.try_push(ci_t(2)));
    REQUIRE(ci_t::inst_cnt == 2);
    s.reset(0);
    REQUIRE(ci_t::inst_cnt == 0);
    REQUIRE_FALSE(s.try_push(ci_t(1)));
  }
}
