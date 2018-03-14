#ifndef LF_ALLOCATOR_HPP
#define LF_ALLOCATOR_HPP

#include "common.hpp"

#include <cstddef>
#include <atomic>
#include <memory>

namespace lf {

namespace allocator_impl {

template <typename T>
struct node {
  T data;
  std::atomic<node*> next;
};

template <typename T>
node<T>* allocate(std::size_t n) {
  return std::allocator<node<T>>{}.allocate(n);
}

template <typename T>
void deallocate(node<T>* p, std::size_t n) noexcept {
  std::allocator<node<T>>{}.deallocate(p, n);
}

template <typename T>
struct nodes_deleter {
  std::size_t n;
  void operator()(node<T>* p) const noexcept {
    deallocate(p, n);
  }
};

} // namespace allocator_impl

template <typename T>
struct deleter;

template <typename T>
class allocator {
  using node = allocator_impl::node<T>;
  using counted_ptr = lf::counted_ptr<node>;
  using nodes_deleter = allocator_impl::nodes_deleter<T>;

public:
  using deleter = lf::deleter<T>;

  // copy control
  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  // construct
  explicit allocator(std::size_t capacity):
    nodes(allocator_impl::allocate<T>(capacity), nodes_deleter{capacity}),
    head({nodes.get()}) {
    auto p = nodes.get();
    auto last = p + capacity - 1;
    while (p < last) {
      p->next.store(p + 1, rlx);
      ++p;
    }
    p->next.store(nullptr, rlx);
  }

  // modifier
  T* allocate(std::size_t = 1) {
    counted_ptr oldhead(head.load(acq)), newhead;
    do {
      if (!oldhead.p) throw std::bad_alloc{};
      newhead.p = oldhead.p->next.load(rlx);
      newhead.trefcnt = oldhead.trefcnt + 1;
    }
    while (!head.compare_exchange_weak(oldhead, newhead, rlx, acq));
    return std::addressof(oldhead.p->data);
  }

  void deallocate(T* p, std::size_t = 1) noexcept {
    auto pn = (node*)p;
    counted_ptr oldhead(head.load(rlx)), newhead{pn};
    do {
      pn->next.store(oldhead.p, rlx);
      newhead.trefcnt = oldhead.trefcnt;
    }
    while (!head.compare_exchange_weak(oldhead, newhead, rel, rlx));
  }

  // observer
  std::size_t capacity() const noexcept {
    return nodes.get_deleter().n;
  }

  deleter get_deleter() const noexcept {
    return { const_cast<allocator&>(*this) };
  }

private:
  std::unique_ptr<node, nodes_deleter> nodes;
  std::atomic<counted_ptr> head;
};
// class allocator

template <typename T>
struct deleter {
  allocator<T>& alloc;
  void operator()(T* p) const noexcept {
    dismiss(alloc, p);
  }
};

} // namespace lf

#endif // LF_ALLOCATOR_HPP
