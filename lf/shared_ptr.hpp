#ifndef LF_SHARED_PTR_HPP
#define LF_SHARED_PTR_HPP

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

namespace lf {

namespace shared_ptr_impl {

template <typename T>
struct block {
  std::unique_ptr<T> pdata;
  std::atomic_uint64_t cnt;
};

} // namespace shared_ptr_impl

template <typename T>
class shared_ptr {
  template <typename U>
  friend class atomic_shared_ptr;

  using unique_ptr = std::unique_ptr<T>;
  using block = shared_ptr_impl::block<T>;

public:
  // copy control
  shared_ptr(const shared_ptr& p):
    pblock(p.pblock) {
    if (pblock) ++pblock->cnt;
  }

  shared_ptr(shared_ptr&& p) noexcept:
    pblock(std::exchange(p.pblock, nullptr)) {
    // pass
  }

  shared_ptr& operator=(shared_ptr p) noexcept {
    swap(*this, p);
    return *this;
  }

 ~shared_ptr() {
    if (pblock) {
      if (--pblock->cnt == 0) delete pblock;
    }
  }

  friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
    std::swap(a.pblock, b.pblock);
  }

  // construct
  shared_ptr(T* p = nullptr):
    pblock(!p ? nullptr : new block{unique_ptr(p), {1}}) {
    // pass
  }

  // modifier
  void reset(T* p = nullptr) {
    auto expire(std::move(*this));
    pblock = !p ? nullptr : new block{unique_ptr(p), {1}};
  }

  // observer
  T* get() const {
    return !pblock ? nullptr : pblock->pdata.get();
  }

  T& operator*() const {
    assert(*this);
    return *pblock->pdata;
  }

  T* operator->() const {
    assert(*this);
    return get();
  }

  explicit operator bool() const {
    return pblock;
  }

private:
  shared_ptr(block* p):
    pblock(p) {
    // pass
  }

  block* pblock;
};

} // namespace lf

#endif // LF_SHARED_PTR_HPP
