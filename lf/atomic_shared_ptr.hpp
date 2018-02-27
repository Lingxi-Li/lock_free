#ifndef LF_ATOMIC_SHARED_PTR_HPP
#define LF_ATOMIC_SHARED_PTR_HPP

#include "common.hpp"
#include "shared_ptr.hpp"

#include <cassert>
#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

template <typename T>
class atomic_shared_ptr {
  using shared_ptr_t = shared_ptr<T>;
  using counted_ptr = counted_ptr<shared_ptr_impl::block<T>>;

public:
  // copy control
  atomic_shared_ptr(const atomic_shared_ptr&) = delete;
  atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
 ~atomic_shared_ptr() {
    auto p = pblock.load();
    if (p.p) {
      p.p->cnt += p.trefcnt;
      shared_ptr_t(p.p);
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
    if (oldp.p) {
      oldp.p->cnt += oldp.trefcnt;
      p.pblock = oldp.p;
    }
  }

  shared_ptr_t exchange(shared_ptr_t p) {
    counted_ptr newp{0, std::exchange(p.pblock, nullptr)};
    auto oldp = pblock.exchange(newp);
    if (oldp.p) oldp.p->cnt += oldp.trefcnt;
    return oldp.p;
  }

  bool compare_exchange_weak(shared_ptr_t& expect, const shared_ptr_t& desire) {
    auto oldp = copy_ptr();
    if (oldp.p != expect.pblock) {
      expect = oldp.p;
      return false;
    }
    counted_ptr newp{0, desire.pblock};
    if (pblock.compare_exchange_strong(oldp, newp)) {
      if (oldp.p) oldp.p->cnt += oldp.trefcnt - 2;
      if (desire) ++desire.pblock->cnt;
      return true;
    }
    if (expect) --expect.pblock->cnt;
    return false;
  }

  bool compare_exchange_weak(shared_ptr_t& expect, shared_ptr_t&& desire) {
    auto oldp = copy_ptr();
    if (oldp.p != expect.pblock) {
      expect = oldp.p;
      return false;
    }
    counted_ptr newp{0, desire.pblock};
    if (pblock.compare_exchange_strong(oldp, newp)) {
      if (oldp.p) oldp.p->cnt += oldp.trefcnt - 2;
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
    return copy_ptr().p;
  }

private:
  mutable std::atomic<counted_ptr> pblock;

  counted_ptr copy_ptr() const {
    counted_ptr p = pblock, pp;
    do {
      if (!p.p) return p;
      pp = p;
      pp.trefcnt += one_trefcnt;
    }
    while (!pblock.compare_exchange_weak(p, pp));
    pp.p->cnt += -one_trefcnt + 1;
    return pp;
  }
};

} // namespace lf

#endif // LF_ATOMIC_SHARED_PTR_HPP
