#ifndef LF_ATOMIC_SHARED_PTR_HPP
#define LF_ATOMIC_SHARED_PTR_HPP

#include "shared_ptr.hpp"

#include <cassert>
#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

namespace atomic_shared_ptr_impl {

using shared_ptr_impl::block;

// Works with block.
// Actual staged reference count = stagecnt / one_stagecnt (i.e., uint64_t(1) << 32).
template <typename T>
struct counted_ptr {
  std::uint64_t stagecnt;
  block<T>* pblock;
};

} // namespace atomic_shared_ptr_impl

template <typename T>
class atomic_shared_ptr {
  using shared_ptr_t = shared_ptr<T>;
  using counted_ptr = atomic_shared_ptr_impl::counted_ptr<T>;
  static constexpr auto one_stagecnt = std::uint64_t(1) << 32;

public:
  // copy control
  atomic_shared_ptr(const atomic_shared_ptr&) = delete;
  atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
 ~atomic_shared_ptr() {
    auto p = pblock.load();
    if (p.pblock) {
      p.pblock->cnt += p.stagecnt;
      shared_ptr_t(p.pblock);
    }
  }

  // construct
  atomic_shared_ptr():
    pblock{} {
    // pass
  }

  atomic_shared_ptr(shared_ptr_t p):
    pblock{0, std::exchange(p.pblock, nullptr)} {
    // pass
  }

  // modify
  void operator=(shared_ptr_t p) {
    counted_ptr newp{0, std::exchange(p.pblock, nullptr)};
    auto oldp = pblock.exchange(newp);
    if (oldp.pblock) {
      oldp.pblock->cnt += oldp.stagecnt;
      p.pblock = oldp.pblock;
    }
  }

  shared_ptr_t exchange(shared_ptr_t p) {
    counted_ptr newp{0, std::exchange(p.pblock, nullptr)};
    auto oldp = pblock.exchange(newp);
    if (oldp.pblock) oldp.pblock->cnt += oldp.stagecnt;
    return oldp.pblock;
  }

  bool compare_exchange_weak(shared_ptr_t& expect, const shared_ptr_t& desire) {
    auto oldp = copy_ptr();
    if (oldp.pblock != expect.pblock) {
      expect = oldp.pblock;
      return false;
    }
    counted_ptr newp{0, desire.pblock};
    if (pblock.compare_exchange_strong(oldp, newp)) {
      if (oldp.pblock) oldp.pblock->cnt += oldp.stagecnt - 2;
      if (desire) ++desire.pblock->cnt;
      return true;
    }
    if (expect) --expect.pblock->cnt;
    return false;
  }

  bool compare_exchange_weak(shared_ptr_t& expect, shared_ptr_t&& desire) {
    auto oldp = copy_ptr();
    if (oldp.pblock != expect.pblock) {
      expect = oldp.pblock;
      return false;
    }
    counted_ptr newp{0, desire.pblock};
    if (pblock.compare_exchange_strong(oldp, newp)) {
      if (oldp.pblock) oldp.pblock->cnt += oldp.stagecnt - 2;
      desire.pblock = nullptr;
      return true;
    }
    if (expect) --expect.pblock->cnt;
    return false;
  }

  bool compare_exchange_strong(shared_ptr_t& expect, const shared_ptr_t& desire) {
    auto p = expect.pblock;
    while (!compare_exchange_weak(expect, desire) && p == expect.pblock);
    return p == expect.pblock;
  }

  bool compare_exchange_strong(shared_ptr_t& expect, shared_ptr_t&& desire) {
    auto p = expect.pblock;
    while (!compare_exchange_weak(expect, std::move(desire)) && p == expect.pblock);
    return p == expect.pblock;
  }

  // observer
  operator shared_ptr_t() const {
    auto p = copy_ptr();
    return p.pblock;
  }

private:
  mutable std::atomic<counted_ptr> pblock;

  counted_ptr copy_ptr() const {
    counted_ptr p = pblock, pp;
    do {
      if (!p.pblock) return p;
      pp = p;
      pp.stagecnt += one_stagecnt;
    }
    while (!pblock.compare_exchange_weak(p, pp));
    pp.pblock->cnt += -one_stagecnt + 1;
    return pp;
  }
};

} // namespace lf

#endif // LF_ATOMIC_SHARED_PTR_HPP
