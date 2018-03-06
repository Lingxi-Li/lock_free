#include "../lf/atomic_shared_ptr.hpp"

#include "common.hpp"

namespace atomic_shared_ptr_impl {

struct stru {
  stru() noexcept { ++cnt; }
 ~stru() { --cnt; }
  static unsigned cnt;
};
unsigned stru::cnt = 0;

}

LF_TEST_BEGIN(atomic_shared_ptr)
  using namespace atomic_shared_ptr_impl;
  using sp_t = lf::shared_ptr<stru>;
  using asp_t = lf::atomic_shared_ptr<stru>;
  {
    auto p1 = new stru, p2 = new stru;
    sp_t sp1(p1);
    {
      asp_t asp1, asp2(std::move(sp1));
      assert(!asp1.load());
      assert(asp2.load().get() == p1);
      asp1.store(sp_t(p2));
      assert(asp1.load().get() == p2);
      assert(stru::cnt == 2);
      auto sp2 = asp1.exchange(sp_t{});
      assert(sp2.get() == p2);
    }
    assert(stru::cnt == 0);
  }
  {
    auto p1 = new stru, p2 = new stru;
    sp_t sp1(p1), sp2(p2), sp3;
    asp_t asp1, asp2;
    assert(!asp1.compare_exchange_strong(sp1, sp2));
    assert(!sp1);
    assert(asp1.compare_exchange_strong(sp1, std::move(sp2)));
    assert(!sp2);
    assert(asp1.load().get() == p2);
  }
  assert(stru::cnt == 0);
LF_TEST_END
