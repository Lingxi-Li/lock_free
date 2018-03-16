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
  stack() noexcept = default;

  // modify
  template <typename... Us>
  bool try_emplace(Us&&... args) {
    auto p = new(std::nothrow) node(std::forward<Us>(args)...);
    if (!p) return false;
    counted_ptr newhead{p};
    p->next = head.load(rlx);
    while (!head.compare_exchange_weak(p->next, newhead, rel, rlx));
    return true;
  }

  template <typename... Us>
  void emplace(Us&&... args) {
    if (!try_emplace(std::forward<Us>(args)...)) {
      throw std::bad_alloc{};
    }
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
        unhold_ptr_rel(oldhead);
        return true;
      }
      else {
        unhold_ptr_acq(p);
      }
    }
  }

private:
  std::atomic<counted_ptr> head{};
};

} // namespace lf

#endif // LF_STACK_HPP
