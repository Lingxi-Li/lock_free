#include "../../lf/allocator.hpp"
#include "../../lf/allocator.hpp"

#include "test.hpp"

namespace {

void require_capacity_2(lf::allocator<int>& allo) {
  auto p1 = allo.try_allocate();
  REQUIRE(p1);
  auto p2 = allo.try_allocate();
  REQUIRE(p2);
  REQUIRE_FALSE(allo.try_allocate());
  allo.deallocate(p1);
  allo.deallocate(p2);
  REQUIRE(allo.try_allocate() == p2);
  REQUIRE(allo.try_allocate() == p1);
  REQUIRE_FALSE(allo.try_allocate());
  allo.deallocate(p2);
  allo.deallocate(p1);
}

} // unnamed namespace

TEST_CASE("allocator") {
  SECTION("ctor") {
    lf::allocator<int> a1, a2(0), a3(2);
    REQUIRE_FALSE(a1.try_allocate());
    REQUIRE_FALSE(a2.try_allocate());
    require_capacity_2(a3);
  }
  SECTION("reset") {
    lf::allocator<int> a;
    REQUIRE_FALSE(a.try_allocate());
    a.reset(2);
    require_capacity_2(a);
    a.reset(0);
    REQUIRE_FALSE(a.try_allocate());
  }
}
