#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include "common.hpp"

#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

namespace stack_impl {

template <typename T>
struct node {
  template <typename... Us>
  node(Us&&... args):
    data(std::forward<Us>(args)...),
    trefcnt(0),
    pnext{} {
    // pass
  }

  T data;
  std::atomic_uint64_t trefcnt;
  counted_ptr<node> pnext;
};

} // namespace stack_impl

template <typename T>
class stack {
  using node = stack_impl::node<T>;
  using counted_ptr = counted_ptr<node>;

public:
  // copy control
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;
 ~stack() {
    auto p = phead.load();
    while (p.p) {
      auto pdel = std::exchange(p, p.p->pnext);
      delete pdel.p;
    }
  }

  // construct
  stack():
    phead{} {
    // pass
  }

  // modify
  template <typename... Us>
  void emplace(Us&&... args) {
    auto p = new node(std::forward<Us>(args)...);
    counted_ptr pnode{0, p};
    p->pnext = phead;
    while (!phead.compare_exchange_weak(p->pnext, pnode));
  }

  bool try_pop(T& val) {
    auto oldp = phead.load();
    while (true) {
      oldp = copy_ptr(oldp);
      auto p = oldp.p;
      if (!p) return false;
      if (phead.compare_exchange_strong(oldp, p->pnext)) {
        val = std::move(p->data);
        if ((p->trefcnt += oldp.trefcnt - 1) == 0) delete p;
        return true;
      }
      else {
        if (--p->trefcnt == 0) delete p;
      }
    }
  }

private:
  mutable std::atomic<counted_ptr> phead;

  counted_ptr copy_ptr(counted_ptr oldp) {
    counted_ptr newp;
    do {
      if (!oldp.p) return oldp;
      newp = oldp;
      ++newp.trefcnt;
    }
    while (!phead.compare_exchange_weak(oldp, newp));
    return newp;
  }
};

} // namespace lf

#endif // LF_STACK_HPP