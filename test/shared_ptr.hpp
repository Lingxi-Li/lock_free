#include "../lf/shared_ptr.hpp"

#include "common.hpp"

namespace shared_ptr_impl {

struct stru {
  stru() noexcept { ++cnt; }
 ~stru() { --cnt; }
  static unsigned cnt;
};
unsigned stru::cnt = 0;

}

LF_TEST_BEGIN(shared_ptr)
  using namespace shared_ptr_impl;
  using sp_t = lf::shared_ptr<stru>;
  auto p1 = new stru, p2 = new stru;
  sp_t sp_0, sp_1(p1), sp_2(p2);
  assert(!sp_0);
  assert(sp_1);
  {
    sp_t sp_3(sp_1), sp_4(std::move(sp_2));
    assert(sp_3.get() == sp_1.get());
    assert(sp_4.get() == p2);
    assert(!sp_2);
    assert(sp_3->cnt == 2);
  }
  assert(sp_1->cnt == 1);
  {
    sp_t sp_3;
    sp_3 = sp_1;
    assert(sp_3.get() == sp_1.get());
  }
  assert((*sp_1).cnt == 1);
  {
    sp_t sp_3;
    sp_3 = std::move(sp_1);
    assert(sp_3.get() == p1);
    assert(!sp_1);
  }
  assert(stru::cnt == 0);

  p1 = new stru, p2 = new stru;
  sp_t sp_3(p1);
  swap(sp_3, sp_1);
  assert(!sp_3 && sp_1);
  sp_1.reset(p2);
  assert(sp_1->cnt == 1);
  assert(sp_1.get() == p2);
  sp_1.reset();
  assert(stru::cnt == 0);
LF_TEST_END
