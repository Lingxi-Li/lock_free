#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

namespace stack_impl {

template <typename T>
struct node;

template <typename T>
struct counted_ptr {
  std::uint64_t extcnt; // https://stackoverflow.com/q/48947428/1348273
  node<T>* pnode;
};

template <typename T>
struct node {
  template <typename... Us>
  node(Us&&... args):
    data(std::forward<Us>(args)...),
    intcnt(0),
    pnext{} {
    // pass
  }

  T data;
  std::atomic_uint64_t intcnt;
  counted_ptr<T> pnext;
};

} // namespace stack_impl

template <typename T>
class stack {
  using counted_ptr = stack_impl::counted_ptr<T>;
  using node = stack_impl::node<T>;

public:
  // copy control
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;
 ~stack() {
    auto p = phead.load();
    while (p.pnode) {
      auto pdel = std::exchange(p, p.pnode->pnext);
      delete pdel.pnode;
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
    counted_ptr pnode{1, p};
    p->pnext = phead;
    while (!phead.compare_exchange_weak(p->pnext, pnode));
  }

  bool pop(T& val) {
    auto oldp = phead.load();
    while (true) {
      oldp = copy_ptr(oldp);
      auto p = oldp.pnode;
      if (!p) return false;
      if (phead.compare_exchange_strong(oldp, p->pnext)) {
        val = std::move(p->data);
        if ((p->intcnt += oldp.extcnt - 2) == 0) delete p;
        return true;
      }
      else {
        if (--p->intcnt == 0) delete p;
      }
    }
  }

private:
  std::atomic<counted_ptr> phead;

  counted_ptr copy_ptr(counted_ptr oldp) {
    counted_ptr newp;
    do {
      if (!oldp.pnode) return oldp;
      newp = oldp;
      ++newp.extcnt;
    }
    while (!phead.compare_exchange_weak(oldp, newp));
    return newp;
  }
};

} // namespace lf

#endif // LF_STACK_HPP