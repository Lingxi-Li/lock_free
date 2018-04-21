#include "../../lf/memory.hpp"
#include "../../lf/memory.hpp"

#include "test.hpp"

#include <cstdint>
#include <atomic>
#include <initializer_list>
#include <vector>

using ci_t = counted<int>;
using veci_t = std::vector<int>;
using upci_t = lf::unique_ptr<ci_t>;

namespace {

struct list_init {
  list_init(std::initializer_list<int>) {}
};

struct stru {
  triple tri;
  veci_t vec;
  std::atomic_int atm;
  list_init lst;
};

void require(veci_t* pvec, triple* ptri, std::atomic_int* patm) {
  REQUIRE(pvec->size() == 2);
  REQUIRE(pvec->at(0) == 1);
  REQUIRE(pvec->at(1) == 1);
  REQUIRE(memcmp(*ptri, triple{1, 2, 3}));
  REQUIRE(*patm == 0);
}

inline constexpr
struct test_init_t {
  template <typename Init>
  void operator()(Init init) const {
    SECTION("lval") {
      auto pvec = alloc<veci_t>();
      auto ptri = alloc<triple>();
      auto patm = alloc<std::atomic_int>();
      auto plst = alloc<list_init>();
      init(pvec, 2, 1);
      init(ptri, 1, 2, 3);
      init(patm);
      init(plst, 1, 2, 3);
      require(pvec, ptri, patm);
      for_each(lf::dismiss, pvec, ptri, patm, plst);
    }

    SECTION("rval") {
      auto p = alloc<stru>();
      init(&p->tri, 1, 2, 3);
      init(&p->vec, 2, 1);
      init(&p->atm);
      init(&p->lst, 1, 2, 3);
      require(&p->vec, &p->tri, &p->atm);
      lf::dismiss(p);
    }
  }
}
test_init;

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
    for_each(test_init, lf::init_no_catch, lf::init);
  }

  SECTION("LF_MAKE") {
    LF_MAKE(pvec, veci_t,, 2, 1);
    LF_MAKE(ptri, triple,, 1, 2, 3);
    LF_MAKE(patm, std::atomic_int,);
    require(pvec, ptri, patm);
    for_each(lf::dismiss, pvec, ptri, patm);
  }

  SECTION("emplace") {
    auto vec = lf::emplace<veci_t>(2, 1);
    auto tri = lf::emplace<triple>(1, 2, 3);
    auto atm = lf::emplace<std::atomic_int>();
    auto lst = lf::emplace<list_init>(1, 2, 3);
    require(&vec, &tri, &atm);
  }

  SECTION("dismiss") {
    LF_MAKE(p0, ci_t,, 0);
    LF_MAKE(p1, ci_t,, 0);
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
      LF_MAKE_UNIQUE(up, ci_t,, 1);
      REQUIRE_SAME_T(decltype(up), upci_t);
      REQUIRE(up);
      REQUIRE(up->cnt == 1);
      REQUIRE(ci_t::inst_cnt == 1);
    }
    REQUIRE(ci_t::inst_cnt == 0);
    LF_MAKE_UNIQUE(up, std::atomic_int,);
    REQUIRE(up);
    REQUIRE(*up == 0);
  }

}
