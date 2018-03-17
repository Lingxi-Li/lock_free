#include "common.hpp"

#include <atomic>
#include <memory>

namespace compiler_impl {

struct stru {
  std::atomic<int> a;
  std::atomic<int> b;
};

struct pr {
  int a;
  int b;
};

pr func(pr v) noexcept {
  return v;
}

} // namespace compiler_impl

LF_TEST_BEGIN(compiler)
  using namespace compiler_impl;
  // zero-initialize atomic
  {
    std::atomic<int> a0{};
    assert(a0 == 0);
    std::unique_ptr<std::atomic<int>> pa(new std::atomic<int>{});
    assert(*pa == 0);
  }
  // initialize aggregate atomic member
  {
    stru s0{{}};
    assert(s0.a == 0);
    assert(s0.b == 0);
    std::unique_ptr<stru> ps(new stru{{}});
    assert(ps->a == 0);
    assert(ps->b == 0);

    stru s1{{1}, {2}};
    assert(s1.a == 1);
    assert(s1.b == 2);
    ps.reset(new stru{{3}, {4}});
    assert(ps->a == 3);
    assert(ps->b == 4);
  }
  {
    auto v = func({});
    assert(v.a == 0 && v.b == 0);
    v = func({{}});
    assert(v.a == 0 && v.b == 0);
    v = func({1, 2});
    assert(v.a == 1 && v.b == 2);
  }
LF_TEST_END
