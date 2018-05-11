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

  explicit stack(std::size_t capacity):
   alloc(capacity) {
    // nop
  }

  ~stack() {
    uninit();
  }

  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  void reset(std::size_t capacity) {
    alloc.reset(capacity, &stack::uninit, this);
    head.store({}, rlx);
  }

  bool try_push(T&& v) noexcept {
    if (auto p = alloc.try_allocate()) {
      init(&p->val, std::move(v));
      cp_t newhd{p}, oldhd(head.load(rlx));
      do {
        p->next.store(oldhd.ptr, rlx);
        newhd.cnt = oldhd.cnt;
      }
      while (!head.compare_exchange_weak(oldhd, newhd, rel, rlx));
      return true;
    }
    else {
      return false;
    }
  }

  std::optional<T> try_pop() noexcept {
    cp_t newhd, oldhd(head.load(acq));
    node* p;
    do {
      if (!(p = oldhd.ptr)) return {};
      newhd.ptr = p->next.load(rlx);
      newhd.cnt = oldhd.cnt + 1;
    }
    while (!head.compare_exchange_weak(oldhd, newhd, rlx, acq));
    auto res = std::make_optional(std::move(p->val));
    alloc.del(p);
    return res;
  }

private:
  using node = typename allocator<T>::node;
  using cp_t = counted_ptr<node>;

  void uninit() noexcept {
    auto p = head.load(rlx).ptr;
    while (p) {
      lf::uninit(&p->val);
      p = p->next.load(rlx);
    }
  }

  allocator<T> alloc;
  std::atomic<cp_t> head{cp_t{}};
};

#include "epilog.inc"

#endif // LF_STACK_HPP
