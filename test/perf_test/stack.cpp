#include "cli.hpp"
#include "libtag.hpp"
#include "simulator2.hpp"

#include <lf/stack.hpp>
#include <boost/lockfree/stack.hpp>

auto val = 0u;

std::vector<simulator2::fn_t> get_lf_fn(std::uint8_t thread_cnt) {
  static lf::stack<unsigned> stk(1_K * thread_cnt * 2);
  for (std::size_t i = 0; i < 1_K * thread_cnt; ++i) {
    stk.try_push(i);
  }
  return {
    []() noexcept {
      (void)stk.try_push(std::move(val));
    },
    []() noexcept {
      (void)stk.try_pop();
    }
  };
}

std::vector<simulator2::fn_t> get_boost_fn(std::uint8_t thread_cnt) {
  static boost::lockfree::stack<unsigned> stk(1_K * thread_cnt * 2);
  for (std::size_t i = 0; i < 1_K * thread_cnt; ++i) {
    stk.push(i);
  }
  return {
    [] {
      (void)stk.push(val);
    },
    [] {
      unsigned ret;
      (void)stk.pop(ret);
    }
  };
}

MAIN(
 lib tag,
 unsigned thread_cnt,
 optional<std::uint16_t, 60> mins) {
  auto get_fn = tag == lib::lf ? &get_lf_fn : &get_boost_fn;
  simulator2::configure(thread_cnt, std::chrono::minutes(mins), get_fn(thread_cnt));
  simulator2::kickoff();
  simulator2::print_results();
}
