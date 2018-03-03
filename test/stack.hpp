#include "../lf/stack.hpp"

#include "common.hpp"

LF_TEST_BEGIN(stack)
  lf::stack<int> stk;
  int v{};
  assert(!stk.try_pop(v) && !v);
  stk.emplace(1);
  stk.emplace(2);
  assert(stk.try_pop(v) && v == 2);
  assert(stk.try_pop(v) && v == 1);
  assert(!stk.try_pop(v) && v == 1);
LF_TEST_END
