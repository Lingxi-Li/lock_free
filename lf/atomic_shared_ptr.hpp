#ifndef LF_ATOMIC_SHARED_PTR_HPP
#define LF_ATOMIC_SHARED_PTR_HPP

#include "common.hpp"
#include "shared_ptr.hpp"

#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

template <typename T>
class atomic_shared_ptr {
  using shared_ptr_t = shared_ptr<T>;
  using counted_ptr = lf::counted_ptr<typename shared_ptr_t::node>;

public:
  // copy control
  atomic_shared_ptr(const atomic_shared_ptr&) = delete;
  atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
 ~atomic_shared_ptr() {
    auto cp = node.load(rlx);
    if (cp.p) {
      cp.p->cnt.fetch_add(cp.trefcnt, rlx);
      shared_ptr_t(cp.p);
    }
  }

  atomic_shared_ptr(shared_ptr_t p = shared_ptr_t{}) noexcept:
    node({std::exchange(p.p, nullptr)}) {
  }

  // modify
  void store(shared_ptr_t p, std::memory_order ord = cst) noexcept {
    counted_ptr newnode{std::exchange(p.p, nullptr)};
    auto oldnode = node.exchange(newnode, ord);
    if (oldnode.p) {
      oldnode.p->cnt.fetch_add(oldnode.trefcnt, rlx);
      p.p = oldnode.p;
    }
  }

  shared_ptr_t exchange(shared_ptr_t p, std::memory_order ord = cst) noexcept {
    counted_ptr newnode{std::exchange(p.p, nullptr)};
    auto oldnode = node.exchange(newnode, ord);
    if (oldnode.p) oldnode.p->cnt.fetch_add(oldnode.trefcnt, rlx);
    return oldnode.p;
  }

  bool compare_exchange_weak(
    shared_ptr_t& expect, const shared_ptr_t& desire,
    std::memory_order success = cst, std::memory_order fail = cst) noexcept {
    return compare_exchange_weak(expect, desire, success, fail,
      [&desire] { if (desire) desire.p->cnt.fetch_add(1, rlx); });
  }

  bool compare_exchange_weak(
    shared_ptr_t& expect, shared_ptr_t&& desire,
    std::memory_order success = cst, std::memory_order fail = cst) noexcept {
    return compare_exchange_weak(expect, desire, success, fail,
      [&desire] { desire.p = nullptr; });
  }

  bool compare_exchange_strong(
    shared_ptr_t& expect, const shared_ptr_t& desire,
    std::memory_order success = cst, std::memory_order fail = cst) noexcept {
    auto p = expect.p;
    while (!compare_exchange_weak(expect, desire, success, fail) && p == expect.p);
    return p == expect.p;
  }

  bool compare_exchange_strong(
    shared_ptr_t& expect, shared_ptr_t&& desire,
    std::memory_order success = cst, std::memory_order fail = cst) noexcept {
    auto p = expect.p;
    while (!compare_exchange_weak(expect, std::move(desire), success, fail) && p == expect.p);
    return p == expect.p;
  }

  // observer
  shared_ptr_t load(std::memory_order ord = cst) const noexcept {
    auto oldnode = node.load(ord);
    if (hold_ptr_if_not_null(node, oldnode, ord, ord)) {
      oldnode.p->cnt.fetch_add(-one_trefcnt + 1, rlx);
    }
    return oldnode.p;
  }

private:
  template <typename CopyOrMove>
  bool compare_exchange_weak(
    shared_ptr_t& expect, const shared_ptr_t& desire,
    std::memory_order success, std::memory_order fail,
    CopyOrMove copy_or_move) noexcept {
    auto oldnode = node.load(fail);
    if (hold_ptr_if_not_null(node, oldnode, fail, fail)) {
      oldnode.p->cnt.fetch_add(-one_trefcnt + 1, rlx);
    }
    if (oldnode.p != expect.p) {
      expect = oldnode.p;
      return false;
    }
    counted_ptr newnode{desire.p};
    if (node.compare_exchange_strong(oldnode, newnode, success, rlx)) {
      copy_or_move();
      if (oldnode.p) oldnode.p->cnt.fetch_add(oldnode.trefcnt - 2, rlx);
      return true;
    }
    if (expect) expect.p->cnt.fetch_sub(1, rlx);
    return false;
  }

  mutable std::atomic<counted_ptr> node;
};

} // namespace lf

#endif // LF_ATOMIC_SHARED_PTR_HPP
