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
    auto p = pdata.load();
    if (p) delete p;
  }

  std::atomic_uint64_t cnt;
  std::atomic<T*> pdata;
  std::atomic_counted_ptr<node> pnext;
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
    auto p = head.load();
    while (p) {
      auto delp = std::exchange(p, p->pnext.p);
      delete delp;
    }
  }

  // construct
  queue():
    head({0, new node{{2}}}),
    tail(head) {
    // pass
  }

  // modifier
  bool try_pop(T& v) {
    auto oldhead = head.load();
    while (true) {
      oldhead = hold_ptr(head, oldhead);
      auto p = oldhead.p;
      if (p == tail.load().p) {
        if ((p->cnt -= one_trefcnt) == 0) delete p;
        return false;
      }
      if (head.compare_exchange_strong(oldhead, p->pnext)) {
        v = std::move(*p->pdata);
        if ((p->cnt += oldhead.trefcnt - one_trefcnt - 1) == 0) delete p;
        return true;
      }
      if ((p->cnt -= one_trefcnt) == 0) delete p;
    }
  }

  template <typename... Us>
  void emplace(Us&&... args) {
    auto pdata = new T{std::forward<Us>(args)...};
    std::unique_ptr<T> pnode(new node{{2}});
    T* null = nullptr;
    auto oldtail = tail.load();
    do {
      oldtail = hold_ptr(tail, oldtail);
      auto p = oldtail.p;
      null = nullptr;
      p->pdata.compare_exchange_strong(null, pdata);
      counted_ptr oldnext{};
      counted_ptr newnext{0, pnode.get()};
      if (!p->pnext.compare_exchange_strong(oldnext, newnext)) {
        newnext = oldnext;
      }
      else {
        pnode.release();
        if (null) pnode.reset(new node{{2}});
      }
      if (tail.compare_exchange_strong(oldtail, newnext)) {
        if (!(p->cnt += oldtail.trefcnt - one_trefcnt - 1)) {
          delete p;
        }
      }
      else {
        if (!(p->cnt -= one_trefcnt)) delete p;
      }
    }
    while (null);
  }

private:
  static counted_ptr hold_ptr(std::atomic<counted_ptr>& stub, counted_ptr oldp) {
    counted_ptr newp;
    do {
      newp = oldp;
      newp.trefcnt += one_trefcnt;
    }
    while (!stub.compare_exchange_weak(oldp, newp));
    return newp;
  }

  std::atomic<counted_ptr> head;
  std::atomic<counted_ptr> tail;
};

} // namespace lf

#endif // LF_QUEUE_HPP