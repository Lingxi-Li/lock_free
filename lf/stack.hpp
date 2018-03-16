#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include "common.hpp"
#include "arg_pack.hpp"

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

  template <LF_ARG_PACK_T(Us, Is)>
  explicit stack(LF_ARG_PACK(Us, Is) alloc_args):
    alloc(LF_UNPACK_ARGS(alloc_args, Is)) {
  }

  // modify
  template <typename... Us>
  void emplace(Us&&... args) {
    auto p = make(alloc, std::forward<Us>(args)...);
    counted_ptr newhead{p};
    p->next = head.load(rlx);
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
        unhold_ptr_rel(oldhead, 0, alloc);
        return true;
      }
      else {
        unhold_ptr_acq(p, alloc);
      }
    }
  }

private:
  Alloc<node> alloc{};
  std::atomic<counted_ptr> head{};
};

} // namespace lf

#endif // LF_STACK_HPP
