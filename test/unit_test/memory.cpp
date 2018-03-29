#include "../../lf/memory.hpp"
#include "../../lf/memory.hpp"

#include "test.hpp"

#include <cstdint>
#include <atomic>
#include <vector>

TEST_CASE("memory") {

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

  SECTION("construct") {
    using veci_t = std::vector<int>;

    SECTION("basic") {
      auto p = lf::allocate<veci_t>();
      lf::construct(p, 2, 1);
      std::unique_ptr<veci_t> up(p);
      REQUIRE(p->size() == 2);
      REQUIRE(p->at(0) == 1);
      REQUIRE(p->at(1) == 1);
    }

    SECTION("aggregate init") {
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
