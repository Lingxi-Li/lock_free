#ifndef LF_SHARED_PTR_HPP
#define LF_SHARED_PTR_HPP

#include "common.hpp"

#include <cstdint>
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

namespace lf {

namespace shared_ptr_impl {

template <typename T>
struct node {
  node(T* data, std::uint64_t cnt):
    data(data), cnt(cnt) {
  }

  std::unique_ptr<T> data;
  std::atomic_uint64_t cnt;
};

} // namespace shared_ptr_impl

template <typename T>
class shared_ptr {
  template <typename U>
  friend class atomic_shared_ptr;

  using node = shared_ptr_impl::node<T>;

public:
  // copy control
  shared_ptr(const shared_ptr& sp) noexcept:
    p(sp.p) {
    if (p) p->cnt.fetch_add(1, rlx);
  }

  shared_ptr(shared_ptr&& sp) noexcept:
    p(std::exchange(sp.p, nullptr)) {
  }

  shared_ptr& operator=(shared_ptr sp) noexcept {
    swap(*this, sp);
    return *this;
  }

 ~shared_ptr() {
    if (p) {
      if (p->cnt.fetch_sub(1, rel) == 1) {
        p->cnt.load(acq);
        delete p;
      }
    }
  }

  friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
    std::swap(a.p, b.p);
  }

  // construct
  shared_ptr() noexcept:
    p{} {
  }

  explicit shared_ptr(T* p):
    p(p ? new node(p, 1) : nullptr) {
  }

  // modifier
  void reset(T* p = nullptr) {
    shared_ptr expire(p);
    swap(*this, expire);
  }

  // observer
  T* get() const noexcept { return p ? p->data.get() : nullptr; }
  T& operator*() const noexcept { return *p->data; }
  T* operator->() const noexcept { return get(); }
  explicit operator bool() const noexcept { return p; }

private:
  shared_ptr(node* p) noexcept:
    p(p) {
  }

  node* p;
};

} // namespace lf

#endif // LF_SHARED_PTR_HPP
