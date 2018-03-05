#ifndef LF_QUEUE_HPP
#define LF_QUEUE_HPP

#include "common.hpp"

#include <atomic>
#include <memory>
#include <utility>

namespace lf {

namespace queue_impl {

template <typename T>
struct node {
 ~node() {
    // delete nullptr is OK
    delete data.load(rlx);
  }

  std::atomic_uint64_t cnt;
  atomic_counted_ptr<node> next;
  std::atomic<T*> data;
};

} // namespace queue_impl

template <typename T>
class queue {
  using node = queue_impl::node<T>;
  using counted_ptr = lf::counted_ptr<node>;

public:
  // copy control
  queue(const queue&) = delete;
  queue& operator=(const queue&) = delete;
 ~queue() {
    auto p = head.load(rlx).p;
    while (p) {
      delete std::exchange(p, p->next.load(rlx).p);
    }
  }

  // construct
  queue():
    head({new node{{2}}}),
    tail(head.load(rlx)) {
  }

  // modifier
  bool try_pop(T& v) {
    auto oldhead = head.load(rlx);
    while (true) {
      hold_ptr(head, oldhead, rlx, rlx);
      auto p = oldhead.p;
      if (p == tail.load(acq).p) {
        unhold_ptr(p, false);
        return false;
      }
      if (head.compare_exchange_strong(oldhead, p->next.load(rlx), rlx, rlx)) {
        v = std::move(*p->data.load(rlx));
        unhold_ptr_rel(p, true);
        return true;
      }
      unhold_ptr(p, false);
    }
  }

  template <typename... Us>
  void emplace(Us&&... args) {
    auto data = new T(std::forward<Us>(args)...);
    std::unique_ptr<node> pn;
    bool done;
    auto oldtail = tail.load(rlx);
    do {
      hold_ptr(tail, oldtail, rlx, rlx);
      auto p = oldtail.p;
      T* null = nullptr;
      done = p->data.compare_exchange_strong(null, data, rlx, rlx);
      if (!pn) pn.reset(new node{{2}});
      counted_ptr oldnext{}, newnext{pn.get()};
      if (p->next.compare_exchange_strong(oldnext, newnext, rlx, rlx)) {
        pn.release();
      }
      else {
        newnext = oldnext;
      }
      if (tail.compare_exchange_strong(oldtail, newnext, rel, rlx)) {
        unhold_ptr(p, true);
        oldtail = newnext;
      }
      else {
        unhold_ptr(p, false);
      }
    }
    while (!done);
  }

private:
  std::atomic<counted_ptr> head;
  std::atomic<counted_ptr> tail;
};

} // namespace lf

#endif // LF_QUEUE_HPP
