#include "../lf/allocator.hpp"

#include "common.hpp"

LF_TEST_BEGIN(allocator)
  using alloc_t = lf::allocator<int>;
  alloc_t alloc(2);
  auto p1 = alloc.allocate(1);
  auto p2 = alloc.allocate(1);
  assert(p1 && p2);
  assert(throw_e<std::bad_alloc>([&alloc] { alloc.allocate(1); }));
  alloc.deallocate(p1, 1);
  alloc.deallocate(p2, 1);
  assert(alloc.allocate(1) == p2);
  assert(alloc.allocate(1) == p1);
  assert(throw_e<std::bad_alloc>([&alloc] { alloc.allocate(1); }));
LF_TEST_END
