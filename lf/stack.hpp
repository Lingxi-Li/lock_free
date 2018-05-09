#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include "allocator.hpp"

#include <optional>

#include "prolog.inc"

template <typename T>
class stack {
public:
  stack() noexcept = default;

  explicit stack(std::size_t capacity):
   alloc(capacity) {
    // nop
  }

  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  ~stack() {
    uninit();
  }

  void reset(std::size_t capacity) {
    uninit();

  }

private:
  using node = typename allocator<T>::node;
  using cp_t = counted_ptr<node>;

  void uninit() noexcept {
    auto p = head.load(rlx).ptr;
    while (p) {
      uninit(&p->val);
      p = p->next.load(rlx);
    }
  }

  allocator<T> alloc;
  std::atomic<cp_t> head{cp_t{}};
};

#include "epilog.inc"

#endif // LF_STACK_HPP
