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
  node* next;
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
struct deleter {
  std::size_t n;
  void operator()(node<T>* p) const noexcept {
    deallocate(p, n);
  }
};

} // namespace allocator_impl

template <typename T>
class allocator {
public:
  // copy control
  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  // construct
  explicit allocator(std::size_t capacity):
    nodes(allocator_impl::allocate<T>(capacity), deleter{capacity}),
    head(nodes.get()) {
    auto p = nodes.get();
    auto last = p + capacity - 1;
    while (p < last) {
      p = p->next = p + 1;
    }
    p->next = nullptr;
  }

  // modifier
  T* allocate(std::size_t = 1) {
    auto oldhead = head.load(acq);
    do {
      if (!oldhead) throw std::bad_alloc{};
    }
    while (!head.compare_exchange_weak(oldhead, oldhead->next, rlx, acq));
    return std::addressof(oldhead->data);
  }

  void deallocate(T* p, std::size_t = 1) noexcept {
    auto pn = (node*)p;
    pn->next = head.load(rlx);
    while (!head.compare_exchange_weak(pn->next, pn, rel, rlx));
  }

  // observer
  std::size_t capacity() const noexcept {
    return nodes.get_deleter().n;
  }

private:
  using node = allocator_impl::node<T>;
  using deleter = allocator_impl::deleter<T>;

  std::unique_ptr<node[], deleter> nodes;
  std::atomic<node*> head;
};

} // namespace lf

#endif // LF_ALLOCATOR_HPP
