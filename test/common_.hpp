#include "../lf/common.hpp"

#include "common.hpp"

#include <memory>

namespace common_impl {

struct stru {
  stru(int a, int b) noexcept: a(a), b(b) { ++cnt; }
 ~stru() { --cnt; }
  int a, b;
  static unsigned cnt;
};
unsigned stru::cnt = 0;

}

LF_TEST_BEGIN(common)
  using stru = common_impl::stru;
  std::allocator<stru> alloc;
  auto p = lf::make(alloc, 1, 2);
  assert(stru::cnt == 1);
  assert(p->a == 1 && p->b == 2);
  lf::dismiss(alloc, p);
  assert(stru::cnt == 0);
LF_TEST_END
