#include "../lf/stack.hpp"
#include "../lf/allocator.hpp"

#include "common.hpp"

LF_TEST_BEGIN(stack)
  {
    lf::stack<int> stk;
    int v{};
    assert(!stk.try_pop(v) && !v);
    stk.emplace(1);
    stk.emplace(2);
    assert(stk.try_pop(v) && v == 2);
    assert(stk.try_pop(v) && v == 1);
    assert(!stk.try_pop(v) && v == 1);
  }
  {
    lf::stack<int, lf::allocator> stk(lf::pack_args(2));
    int v{};
    assert(!stk.try_pop(v) && !v);
    stk.emplace(1);
    stk.emplace(2);
    assert(stk.try_pop(v) && v == 2);
    assert(stk.try_pop(v) && v == 1);
    assert(!stk.try_pop(v) && v == 1);
  }
  {
    lf::stack<int, lf::allocator> stk(lf::pack_args(1));
    stk.emplace(1);
    assert(throw_e<std::bad_alloc>([&stk] { stk.emplace(2); }));
  }
LF_TEST_END
