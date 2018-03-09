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
    cnt{} {
  }

  T data;
  counted_ptr<node> next;
  std::atomic_uint64_t cnt;
};

} // namespace stack_impl

template <typename T, template <typename> class Alloc = std::allocator>
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
      dismiss(alloc, std::exchange(p, p->next.p));
    }
  }

  // construct
  stack() = default;

  template <typename... Us>
  explicit stack(Us&&... us):
    alloc(std::forward<Us>(us)...) {
  }

  // modify
  template <typename... Us>
  void emplace(Us&&... args) {
    auto p = make(alloc, std::forward<Us>(args)...);
    p->next = head.load(rlx);
    counted_ptr newhead{p};
    while (!head.compare_exchange_weak(p->next, newhead, rel, rlx));
  }

  bool try_pop(T& val) noexcept {
    auto oldhead = head.load(rlx);
    while (true) {
      if (!hold_ptr_if_not_null(head, oldhead, acq, rlx)) {
        return false;
      }
      auto p = oldhead.p;
      if (head.compare_exchange_strong(oldhead, p->next, rlx, rlx)) {
        val = std::move(p->data);
        unhold_ptr_rel(p, false, alloc);
        return true;
      }
      else {
        unhold_ptr(p, false, alloc);
      }
    }
  }

private:
  Alloc<node> alloc{};
  std::atomic<counted_ptr> head{};
};

} // namespace lf

#endif // LF_STACK_HPP
