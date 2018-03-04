#include "../lf/queue.hpp"

#include "common.hpp"

LF_TEST_BEGIN(queue)
  lf::queue<int> que;
  int v{};
  assert(!que.try_pop(v) && !v);
  que.emplace(1);
  que.emplace(2);
  assert(que.try_pop(v) && v == 1);
  assert(que.try_pop(v) && v == 2);
  assert(!que.try_pop(v) && v == 2);
LF_TEST_END
