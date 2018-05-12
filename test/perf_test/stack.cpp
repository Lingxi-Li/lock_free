#include "cli.hpp"
#include "simulator.hpp"

#include "../../lf/stack.hpp"
#include "boost/lockfree/stack.hpp"

MAIN(unsigned thread_cnt) {
  simulator::configure(thread_cnt, 1_M, 1_K, {});

  // boost::lockfree::stack<int> stk(1_M);
  // for (auto i = 0; i < 1_M; ++i) {
  //   stk.bounded_push(std::move(i));
  // }

  // int v;
  // stk.pop(v);
  // stk.pop(v);
  // std::cout << v << std::endl;

  lf::stack<int> stk(1_M);
  for (auto i = 0; i < 1_M; ++i) {
    stk.try_push(std::move(i));
  }

  stk.try_pop();
  std::cout << *stk.try_pop() << std::endl;
}
