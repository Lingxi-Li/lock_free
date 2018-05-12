#include "../../lf/utility.hpp"
#include "../../lf/utility.hpp"

#include "test.hpp"

TEST_CASE("utility") {
  SECTION("memory order shorthand") {
    REQUIRE_SAME_T(decltype(lf::rlx), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::rel), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::eat), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::cst), const std::memory_order);
    REQUIRE_SAME_T(decltype(lf::acq_rel), const std::memory_order);
    static_assert(lf::rlx == std::memory_order_relaxed);
    static_assert(lf::rel == std::memory_order_release);
    static_assert(lf::acq == std::memory_order_acquire);
    static_assert(lf::eat == std::memory_order_consume);
    static_assert(lf::cst == std::memory_order_seq_cst);
    static_assert(lf::acq_rel == std::memory_order_acq_rel);
  }
  SECTION("cp_t") {
    auto zero = lf::null + 1;
    REQUIRE(zero == 0);
    lf::cp_t cp;
    REQUIRE(cp.ptr == lf::null);
    REQUIRE(cp.cnt == 0);
    REQUIRE(std::atomic<lf::cp_t>(cp).is_lock_free());
  }
}
