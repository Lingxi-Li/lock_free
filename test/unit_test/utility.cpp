#include "../../lf/utility.hpp"
#include "../../lf/utility.hpp"

#include "test.hpp"

TEST_CASE("utility") {

  SECTION("constants") {
    REQUIRE_SAME_T(decltype(lf::rlx), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::rel), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::eat), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::cst), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq_rel), const std::memory_order);
    static_assert(lf::rlx == std::memory_order_relaxed, "");
    static_assert(lf::rel == std::memory_order_release, "");
    static_assert(lf::acq == std::memory_order_acquire, "");
    static_assert(lf::eat == std::memory_order_consume, "");
    static_assert(lf::cst == std::memory_order_seq_cst, "");
    static_assert(lf::acq_rel == std::memory_order_acq_rel, "");

    REQUIRE_SAME_T(decltype(lf::ext_cnt), const std::uint64_t);
    static_assert((lf::ext_cnt >> 32) == 1, "");
  }

  using cpi_t = lf::counted_ptr<int>;
  using acpi_t = lf::atomic_counted_ptr<int>;

  SECTION("counted_ptr") {
    SECTION("default init") {
      auto p = alloc<cpi_t>();
      new(p.get()) cpi_t;
      REQUIRE(p->ptr == nullptr);
      REQUIRE(p->cnt == 0);
    }
    SECTION("atomic_counted_ptr") {
      CHECK(acpi_t{}.is_lock_free());
    }
  }

  SECTION("hold_ptr") {
    auto p = std::make_unique<int>();
    acpi_t null{}, one(cpi_t{p.get(), 1});
    auto ori_null = null.load();
    auto ori_one = one.load();
    SECTION("hold_ptr") {
      hold_ptr(null, ori_null, lf::rlx);
      REQUIRE(memcmp(null.load(), ori_null));
      REQUIRE(ori_null.ptr == nullptr);
      REQUIRE(ori_null.cnt == lf::ext_cnt);

      hold_ptr(one, ori_one, lf::rlx);
      REQUIRE(memcmp(one.load(), ori_one));
      REQUIRE(ori_one.ptr == p.get());
      REQUIRE(ori_one.cnt == lf::ext_cnt + 1);
    }
    SECTION("hold_ptr_if_not_null") {
      REQUIRE_FALSE(hold_ptr_if_not_null(null, ori_null, lf::rlx));
      REQUIRE(memcmp(null.load(), ori_null));
      REQUIRE(ori_null.ptr == nullptr);
      REQUIRE(ori_null.cnt == 0);

      REQUIRE(hold_ptr_if_not_null(one, ori_one, lf::rlx));
      REQUIRE(memcmp(one.load(), ori_one));
      REQUIRE(ori_one.ptr == p.get());
      REQUIRE(ori_one.cnt == lf::ext_cnt + 1);
    }
  }

  using node = counted<>;
  int val = 0;
  auto del = [&val](node* p) noexcept {
    ++val;
    delete p;
  };

  SECTION("unhold_ptr T*") {
    auto plast = new node(lf::ext_cnt);
    auto prem = std::make_unique<node>(lf::ext_cnt + 1);
    REQUIRE(node::inst_cnt == 2);

    SECTION("default deleter") {
      lf::unhold_ptr_acq(plast);
      REQUIRE(node::inst_cnt == 1);
      lf::unhold_ptr_acq(prem.get());
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(prem->cnt == 1);
    }
    SECTION("custom deleter") {
      lf::unhold_ptr_acq(plast, del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      lf::unhold_ptr_acq(prem.get(), del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      REQUIRE(prem->cnt == 1);
    }
  }

  using cpn_t = lf::counted_ptr<node>;

  SECTION("unhold_ptr counted_ptr") {
    cpn_t last{new node(1), lf::ext_cnt};
    auto prem = std::make_unique<node>(2);
    cpn_t rem{prem.get(), 2 * lf::ext_cnt};
    REQUIRE(node::inst_cnt == 2);

    SECTION("acq default deleter") {
      unhold_ptr_acq(last, 1);
      REQUIRE(node::inst_cnt == 1);
      unhold_ptr_acq(rem, 1);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(prem->cnt == lf::ext_cnt + 1);
    }

    SECTION("rel default deleter") {
      unhold_ptr_rel(last, 1);
      REQUIRE(node::inst_cnt == 1);
      unhold_ptr_rel(rem, 1);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(prem->cnt == lf::ext_cnt + 1);
    }

    SECTION("acq custom deleter") {
      unhold_ptr_acq(last, 1, del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      unhold_ptr_acq(rem, 1, del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      REQUIRE(prem->cnt == lf::ext_cnt + 1);
    }

    SECTION("rel custom deleter") {
      unhold_ptr_rel(last, 1, del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      unhold_ptr_rel(rem, 1, del);
      REQUIRE(node::inst_cnt == 1);
      REQUIRE(val == 1);
      REQUIRE(prem->cnt == lf::ext_cnt + 1);
    }
  }
}
