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
    next{},
    trefcnt{} {
  }

  T data;
  counted_ptr<node> next;
  std::atomic_uint64_t trefcnt;
};

} // namespace stack_impl

template <typename T>
class stack {
  using node = stack_impl::node<T>;
  using counted_ptr = lf::counted_ptr<node>;

public:
  // copy control
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;
 ~stack() {
    auto p = head.load(rlx).p;
    while (p) {
      delete std::exchange(p, p->next.p);
    }
  }

  // construct
  stack():
    head{} {
  }

  // modify
  template <typename... Us>
  void emplace(Us&&... args) {
    auto p = new node(std::forward<Us>(args)...);
    p->next = head.load(rlx);
    counted_ptr newhead{p};
    while (!head.compare_exchange_weak(p->next, newhead, rel, rlx));
  }

  bool try_pop(T& val) {
    auto oldhead = head.load(rlx);
    while (true) {
      hold_ptr(oldhead);
      auto p = oldhead.p;
      if (!p) return false;
      if (head.compare_exchange_strong(oldhead, p->next, rlx, rlx)) {
        val = std::move(p->data);
        auto diff = oldhead.trefcnt - 1;
        if (p->trefcnt.fetch_add(diff, rel) == diff) {
          delete p;
        }
        return true;
      }
      else {
        if (p->trefcnt.fetch_sub(1, rlx) == 1) {
          p->trefcnt.load(acq);
          delete p;
        }
      }
    }
  }

private:
  void hold_ptr(counted_ptr& old) {
    counted_ptr ne;
    do {
      if (!old.p) return;
      ne = old;
      ++ne.trefcnt;
    }
    while (!head.compare_exchange_weak(old, ne, acq, rlx));
    old = ne;
  }

  std::atomic<counted_ptr> head;
};

} // namespace lf

#endif // LF_STACK_HPP