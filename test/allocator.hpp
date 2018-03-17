#include "../lf/allocator.hpp"

#include "common.hpp"

#include <type_traits>

namespace allocator_impl {

struct stru {
  stru() noexcept { ++cnt; }
 ~stru() { --cnt; }
  static int cnt;
};
int stru::cnt = 0;

} // namespace allocator_impl

LF_TEST_BEGIN(allocator)
  using namespace allocator_impl;
  using alloc_t = lf::allocator<stru>;
  {
    alloc_t alloc(2);
    auto p1 = alloc.try_allocate();
    auto p2 = alloc.try_allocate();
    assert(p1 && p2);
    assert(!alloc.try_allocate());
    assert(stru::cnt == 0);
    alloc.deallocate(p1);
    alloc.deallocate(p2);
    assert(stru::cnt == 0);
    assert(alloc.try_allocate() == p2);
    assert(alloc.try_allocate() == p1);
    assert(!alloc.try_allocate());
    assert(stru::cnt == 0);
    auto deleter = alloc.get_deleter();
    LF_TEST_SAME_T(decltype(deleter), alloc_t::deleter);
    deleter(p2);
    deleter(p1);
    assert(stru::cnt == -2);
    assert(alloc.try_allocate() == p1);
    assert(alloc.try_allocate() == p2);
    assert(!alloc.try_allocate());
    assert(stru::cnt == -2);
    alloc.reset(3);
    p1 = alloc.try_allocate();
    p2 = alloc.try_allocate();
    assert(alloc.try_allocate());
    assert(!alloc.try_allocate());
    assert(stru::cnt == -2);
  }
  {
    alloc_t alloc;
    assert(!alloc.try_allocate());
    alloc.reset(1);
    auto p = alloc.try_allocate();
    assert(p && !alloc.try_allocate());
    alloc.deallocate(p);
    assert(alloc.try_allocate() && !alloc.try_allocate());
  }
  {
    stru::cnt = 0;
    alloc_t alloc(2);
    auto pn = alloc.try_allocate_node();
    LF_TEST_SAME_T(decltype(pn), alloc_t::node*);
    auto pn2 = alloc.try_allocate_node();
    assert(pn && pn2);
    assert(!alloc.try_allocate_node());
    assert(stru::cnt == 0);
    alloc.deallocate(pn);
    alloc.deallocate(pn2);
    assert(stru::cnt == 0);
    assert(alloc.try_allocate_node() == pn2);
    assert(alloc.try_allocate_node() == pn);
    assert(!alloc.try_allocate_node());
    assert(stru::cnt == 0);
    auto deleter = alloc.get_deleter();
    deleter(pn2);
    deleter(pn);
    assert(stru::cnt == -2);
    assert(alloc.try_allocate_node() == pn);
    assert(alloc.try_allocate_node() == pn2);
    assert(!alloc.try_allocate_node());
    LF_TEST_SAME_T(decltype(deleter)::node, alloc_t::node);
  }
LF_TEST_END
