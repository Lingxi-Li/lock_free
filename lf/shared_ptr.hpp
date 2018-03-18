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

template <typename T, typename Deleter>
struct block {
  std::unique_ptr<T, Deleter> ptr;
  std::atomic_uint64_t cnt;
};

} // namespace shared_ptr_impl

template <typename T, typename Deleter>
class atomic_shared_ptr;

template <typename T, typename Deleter = std::default_delete<T>>
class shared_ptr {
  friend class atomic_shared_ptr<T, Deleter>;
  using block = shared_ptr_impl::block<T, Deleter>;

public:
  using unique_ptr = std::unique_ptr<T, Deleter>;

  // copy control
  shared_ptr(const shared_ptr& sp) noexcept:
    pblock(sp.pblock) {
    if (pblock) pblock->cnt.fetch_add(1, rlx);
  }

  shared_ptr(shared_ptr&& sp) noexcept:
    pblock(std::exchange(sp.pblock, nullptr)) {
  }

  shared_ptr& operator=(shared_ptr sp) noexcept {
    swap(*this, sp);
    return *this;
  }

 ~shared_ptr() {
    if (pblock) {
      if (pblock->cnt.fetch_sub(1, rel) == 1) {
        pblock->cnt.load(acq);
        delete pblock;
      }
    }
  }

  friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
    std::swap(a.pblock, b.pblock);
  }

  // construct
  shared_ptr() noexcept = default;

  explicit shared_ptr(unique_ptr&& p):
    pblock(p ? new node{std::move(p), 1} : nullptr) {
  }

  // modifier
  void reset() noexcept {
    shared_ptr{std::move(*this)};
  }

  void reset(unique_ptr&& p) {
    shared_ptr exp(std::move(p));
    swap(*this, exp);
  }

  // observer
  T* get() const noexcept { return pblock ? pblock->ptr.get() : nullptr; }
  T& operator*() const noexcept { return *pblock->ptr; }
  T* operator->() const noexcept { return get(); }
  explicit operator bool() const noexcept { return pblock; }

private:
  shared_ptr(block* p) noexcept:
    pblock(p) {
  }

  block* pblock{};
};

} // namespace lf

#endif // LF_SHARED_PTR_HPP
