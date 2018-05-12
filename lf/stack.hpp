#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include "allocator.hpp"

#include <optional>

#include "prolog.inc"

template <typename T>
class stack {
  static_assert(std::is_move_constructible_v<T>);

public:
  stack() noexcept = default;

  explicit stack(std::uint32_t capacity):
   alloc(capacity) {
    // nop
  }

  ~stack() {
    uninit();
  }

  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  void reset(std::uint32_t capacity) {
    alloc.reset(capacity, &stack::uninit, this);
    head.store({}, rlx);
  }

  bool try_push(T&& v) noexcept {
    auto p = alloc.try_allocate();
    if (p == null) return false;
    auto& nod = alloc.deref(p);
    init(&nod.val, std::move(v));
    cp_t newhd{p}, oldhd(head.load(rlx));
    do {
      nod.next.store(oldhd.ptr, rlx);
      newhd.cnt = oldhd.cnt;
    }
    while (!head.compare_exchange_weak(oldhd, newhd, rel, rlx));
    return true;
  }

  std::optional<T> try_pop() noexcept {
    cp_t newhd, oldhd(head.load(acq));
    node* p;
    do {
      if (oldhd.ptr == null) return {};
      p = &alloc.deref(oldhd.ptr);
      newhd.ptr = p->next.load(rlx);
      newhd.cnt = oldhd.cnt + 1;
    }
    while (!head.compare_exchange_weak(oldhd, newhd, rlx, acq));
    auto res = std::make_optional(std::move(p->val));
    alloc.del(oldhd.ptr);
    return res;
  }

private:
  using node = typename allocator<T>::node;
  using cp_t = counted_ptr;

  void uninit() noexcept {
    auto p = head.load(rlx).ptr;
    while (p != null) {
      auto& nod = alloc.deref(p);
      lf::uninit(&nod.val);
      p = nod.next.load(rlx);
    }
  }

  allocator<T> alloc;
  std::atomic<cp_t> head{cp_t{}};
};

#include "epilog.inc"

#endif // LF_STACK_HPP
