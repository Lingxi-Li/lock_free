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
struct mem {
  using node = node<T>;

  static node* allocate(std::size_t n) {
    return (node*)operator new(sizeof(node) * n);
  }

  struct deallocator {
    void operator()(node* p) const noexcept {
      operator delete(p);
    }
  };
};

} // namespace allocator_impl

template <typename T>
struct deleter;

template <typename T>
class allocator {
  using mem = allocator_impl::mem<T>;
  using counted_ptr = lf::counted_ptr<typename mem::node>;
  using deallocator = typename mem::deallocator;

public:
  using deleter = lf::deleter<T>;
  using node = typename mem::node;

  // copy control
  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  // construct
  allocator() noexcept = default;

  explicit allocator(std::size_t capacity):
    nodes(mem::allocate(capacity)),
    head({nodes.get()}) {
    link_up(capacity);
  }

  // modifier
  void reset(std::size_t capacity) {
    nodes.reset(mem::allocate(capacity));
    head.store({nodes.get()}, rlx);
    link_up(capacity);
  }

  node* try_allocate_node() noexcept {
    counted_ptr oldhead(head.load(acq)), newhead;
    do {
      if (!oldhead.p) return nullptr;
      newhead.p = oldhead.p->next.load(rlx);
      newhead.trefcnt = oldhead.trefcnt + 1;
    }
    while (!head.compare_exchange_weak(oldhead, newhead, rlx, acq));
    return oldhead.p;
  }

  T* try_allocate() noexcept {
    return (T*)try_allocate_node();
  }

  void deallocate(node* pn) noexcept {
    counted_ptr oldhead(head.load(rlx)), newhead{pn};
    do {
      pn->next.store(oldhead.p, rlx);
      newhead.trefcnt = oldhead.trefcnt;
    }
    while (!head.compare_exchange_weak(oldhead, newhead, rel, rlx));
  }

  void deallocate(T* p) noexcept {
    deallocate((node*)p);
  }

  deleter get_deleter() const noexcept {
    return {const_cast<allocator&>(*this)};
  }

private:
  void link_up(std::size_t capacity) noexcept {
    auto p = nodes.get();
    auto last = p + capacity - 1;
    while (p < last) {
      p->next.store(p + 1, rlx);
      ++p;
    }
    p->next.store(nullptr, rlx);
  }

  std::unique_ptr<node, deallocator> nodes{};
  std::atomic<counted_ptr> head{};
};
// class allocator

template <typename T>
struct deleter {
  using node = typename allocator<T>::node;
  allocator<T>& alloc;

  void operator()(node* p) const noexcept {
    operator()((T*)p);
  }

  void operator()(T* p) const noexcept {
    p->~T();
    alloc.deallocate(p);
  }
};

} // namespace lf

#endif // LF_ALLOCATOR_HPP
