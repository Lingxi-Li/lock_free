#include "../../lf/stack.hpp"
#include "../../lf/stack.hpp"

#include "test.hpp"

using ci_t = counted<int>;

TEST_CASE("stack") {

  SECTION("class basic") {
    {
      lf::stack<ci_t> stk;
      REQUIRE(!stk.try_pop());
      stk.emplace(1);
      REQUIRE(ci_t::inst_cnt == 1);
      stk.emplace(2);
      REQUIRE(ci_t::inst_cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 2);
      REQUIRE(ci_t::inst_cnt == 1);
      REQUIRE(stk.try_pop().value().cnt == 1);
      REQUIRE(ci_t::inst_cnt == 0);
      ci_t a(3), b(4);
      REQUIRE(ci_t::inst_cnt == 2);
      stk.emplace(a);
      REQUIRE(ci_t::inst_cnt == 3);
      stk.emplace(std::move(b));
      REQUIRE(ci_t::inst_cnt == 3);
    }
    REQUIRE(ci_t::inst_cnt == 0);
  }

  ci_t ci[]{1, 2};

  SECTION("class bulk ctor") {
    SECTION("copy") {
      lf::stack<ci_t> stk(ci, ci + 2);
      REQUIRE(ci_t::inst_cnt == 4);
      REQUIRE(stk.try_pop().value().cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 1);
      REQUIRE(!stk.try_pop());
      REQUIRE(ci_t::inst_cnt == 2);
    }
    SECTION("move") {
      lf::stack<ci_t> stk(
        std::make_move_iterator(ci),
        std::make_move_iterator(ci) + 2);
      REQUIRE(ci_t::inst_cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 1);
      REQUIRE(!stk.try_pop());
      REQUIRE(ci_t::inst_cnt == 0);
    }
  }

  SECTION("class bulk push") {
    lf::stack<ci_t> stk;
    SECTION("copy") {
      stk.bulk_push(ci, ci + 2);
      REQUIRE(ci_t::inst_cnt == 4);
      REQUIRE(stk.try_pop().value().cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 1);
      REQUIRE(!stk.try_pop());
      REQUIRE(ci_t::inst_cnt == 2);
    }
    SECTION("move") {
      stk.bulk_push(
        std::make_move_iterator(ci),
        std::make_move_iterator(ci) + 2);
      REQUIRE(ci_t::inst_cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 2);
      REQUIRE(stk.try_pop().value().cnt == 1);
      REQUIRE(!stk.try_pop());
      REQUIRE(ci_t::inst_cnt == 0);
    }
  }

  constexpr triple t123{1, 2, 3}, t456{4, 5, 6};

  SECTION("aggregate basic") {
    lf::stack<triple> stk;
    REQUIRE(!stk.try_pop());
    stk.emplace(1, 2, 3);
    stk.emplace(4, 5, 6);
    REQUIRE(memcmp(stk.try_pop().value(), t456));
    REQUIRE(memcmp(stk.try_pop().value(), t123));
  }

  constexpr triple ts[]{t123, t456};

  SECTION("aggregate bulk") {
    lf::stack<triple> stk(ts, ts + 2);
    REQUIRE(memcmp(stk.try_pop().value(), t456));
    REQUIRE(memcmp(stk.try_pop().value(), t123));
    REQUIRE(!stk.try_pop());
    stk.bulk_push(ts, ts + 2);
    REQUIRE(memcmp(stk.try_pop().value(), t456));
    REQUIRE(memcmp(stk.try_pop().value(), t123));
    REQUIRE(!stk.try_pop());
  }

}
