// #include "../../lf/allocator.hpp"
// #include "../../lf/allocator.hpp"

// #include "test.hpp"

// namespace {

// void require_capacity_2(lf::allocator<int>& allo) {
//   auto p1 = allo.try_allocate();
//   REQUIRE(p1);
//   auto p2 = allo.try_allocate();
//   REQUIRE(p2);
//   REQUIRE_FALSE(allo.try_allocate());
//   allo.deallocate(p1);
//   allo.deallocate(p2);
//   REQUIRE(allo.try_allocate() == p2);
//   REQUIRE(allo.try_allocate() == p1);
//   REQUIRE_FALSE(allo.try_allocate());
//   allo.deallocate(p2);
//   allo.deallocate(p1);
// }

// } // unnamed namespace

// TEST_CASE("allocator") {
//   SECTION("ctor") {
//     lf::allocator<int> a1, a2(0), a3(2);
//     REQUIRE_FALSE(a1.try_allocate());
//     REQUIRE_FALSE(a2.try_allocate());
//     require_capacity_2(a3);
//   }
//   SECTION("reset") {
//     lf::allocator<int> a;
//     REQUIRE_FALSE(a.try_allocate());
//     a.reset(2);
//     require_capacity_2(a);
//     a.reset(0);
//     REQUIRE_FALSE(a.try_allocate());
//   }
//   SECTION("reset 2") {
//     auto v = 0;
//     auto inc_fn = [](auto& v) noexcept { ++v; };
//     lf::allocator<int> a;
//     REQUIRE_FALSE(a.try_allocate());
//     a.reset(2, inc_fn, v);
//     REQUIRE(v == 1);
//     require_capacity_2(a);
//     a.reset(0, inc_fn, v);
//     REQUIRE(v == 2);
//     REQUIRE_FALSE(a.try_allocate());
//   }
//   SECTION("make/del") {
//     using ci_t = counted<int>;
//     lf::allocator<ci_t> alloc(2);
//     auto p = alloc.try_make(1);
//     REQUIRE(p);
//     p->val.require(1, 1);
//     auto pp = alloc.try_make(2);
//     REQUIRE(pp);
//     pp->val.require(2, 2);
//     REQUIRE_FALSE(alloc.try_make(3));
//     alloc.del(p);
//     REQUIRE(ci_t::inst_cnt == 1);
//     alloc.del(pp);
//     REQUIRE(ci_t::inst_cnt == 0);
//     REQUIRE(alloc.try_make(3) == pp);
//     pp->val.require(3, 1);
//     REQUIRE(alloc.try_make(4) == p);
//     p->val.require(4, 2);
//     REQUIRE_FALSE(alloc.try_make(5));
//   }
// }
