#include "../../lf/allocator.hpp"
#include "../../lf/allocator.hpp"

#include "test.hpp"

namespace {

void require_capacity_2(lf::allocator<int>& allo) {
  auto p1 = allo.try_allocate();
  REQUIRE(p1 == 0);
  auto p2 = allo.try_allocate();
  REQUIRE(p2 == 1);
  REQUIRE(allo.try_allocate() == lf::null);
  allo.deallocate(p1);
  allo.deallocate(p2);
  REQUIRE(allo.try_allocate() == 1);
  REQUIRE(allo.try_allocate() == 0);
  REQUIRE(allo.try_allocate() == lf::null);
  allo.deallocate(p2);
  allo.deallocate(p1);
}

} // unnamed namespace

TEST_CASE("allocator") {
  SECTION("ctor") {
    lf::allocator<int> a1, a2(0), a3(2);
    REQUIRE(a1.try_allocate() == lf::null);
    REQUIRE(a2.try_allocate() == lf::null);
    require_capacity_2(a3);
  }
  SECTION("reset") {
    lf::allocator<int> a;
    REQUIRE(a.try_allocate() == lf::null);
    a.reset(2);
    require_capacity_2(a);
    a.reset(0);
    REQUIRE(a.try_allocate() == lf::null);
  }
  SECTION("reset 2") {
    auto v = 0;
    auto inc_fn = [](auto& v) noexcept { ++v; };
    lf::allocator<int> a;
    REQUIRE(a.try_allocate() == lf::null);
    a.reset(2, inc_fn, v);
    REQUIRE(v == 1);
    require_capacity_2(a);
    a.reset(0, inc_fn, v);
    REQUIRE(v == 2);
    REQUIRE(a.try_allocate() == lf::null);
  }
  SECTION("deref/del") {
    using ci_t = counted<int>;
    lf::allocator<ci_t> allo(2);
    auto p1 = allo.try_allocate();
    auto p2 = allo.try_allocate();
    REQUIRE(allo.try_allocate() == lf::null);
    auto& nod1 = allo.deref(p1);
    auto& nod2 = allo.deref(p2);
    auto ptrdiff = &nod2 - &nod1;
    REQUIRE(ptrdiff == 1);
    lf::init(&nod1.val, 1);
    lf::init(&nod2.val, 2);
    REQUIRE(ci_t::inst_cnt == 2);
    allo.del(p1);
    allo.del(p2);
    REQUIRE(ci_t::inst_cnt == 0);
    REQUIRE(allo.try_allocate() == p2);
    REQUIRE(allo.try_allocate() == p1);
    REQUIRE(allo.try_allocate() == lf::null);
  }
}
