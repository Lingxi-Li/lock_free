#include "cli.hpp"
#include "libtag.hpp"
#include "simulator.hpp"

#include <lf/stack.hpp>
#include <boost/lockfree/stack.hpp>

std::vector<measure_fn> get_lf_fn(unsigned op_cnt, unsigned margin) {
  auto len = op_cnt + 2 * margin;
  static lf::stack<unsigned> stk(2 * len);
  for (auto i = 0u; i < len; ++i) {
    stk.try_push(std::move(i));
  }
  return {
    []() noexcept {
      auto di = tick::now();
      stk.try_push(0u);
      auto da = tick::now();
      return tp_pr{di, da};
    },
    []() noexcept {
      auto di = tick::now();
      (void)stk.try_pop();
      auto da = tick::now();
      return tp_pr{di, da};
    }
  };
}

std::vector<measure_fn> get_boost_fn(unsigned op_cnt, unsigned margin) {
  auto len = op_cnt + 2 * margin;
  static boost::lockfree::stack<unsigned> stk(2 * len);
  for (auto i = 0u; i < len; ++i) {
    stk.bounded_push(i);
  }
  return {
    [] {
      auto di = tick::now();
      stk.bounded_push(0u);
      auto da = tick::now();
      return tp_pr{di, da};
    },
    [] {
      auto di = tick::now();
      unsigned ret;
      (void)stk.pop(ret);
      auto da = tick::now();
      return tp_pr{di, da};
    }
  };
}

MAIN(
 lib tag,
 unsigned thread_cnt,
 optional<unsigned, 10_M> op_cnt,
 optional<unsigned,  1_M> margin) {
  auto get_fn = tag == lib::lf ? &get_lf_fn : &get_boost_fn;
  auto file_name = "stack_" + to_str(tag) + ".csv";

  simulator::configure(thread_cnt, op_cnt, margin, get_fn(op_cnt, margin));
  simulator::kickoff();
  simulator::write_result(file_name);
}
