#ifndef LF_ATOMIC_SHARED_PTR_HPP
#define LF_ATOMIC_SHARED_PTR_HPP

#include "shared_ptr.hpp"

#include <cassert>
#include <cstdint>
#include <atomic>
#include <utility>

namespace lf {

namespace atomic_shared_ptr_impl {

using shared_ptr_impl::block;

static constexpr auto one_stagecnt = std::uint64_t(1) << 32;

// Works with block.
// Actual staged reference count = stagecnt / one_stagecnt.
template <typename T>
struct counted_ptr {
  std::uint64_t stagecnt;
  block<T>* pblock;
};

} // namespace atomic_shared_ptr_impl

template <typename T>
class atomic_shared_ptr {
  using shared_ptr_t = shared_ptr<T>;
  using counted_ptr = atomic_shared_ptr_impl::counted_ptr<T>;

public:
  // copy control
  atomic_shared_ptr(const atomic_shared_ptr&) = delete;
  atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
 ~atomic_shared_ptr() {
    auto p = pblock.load();
    if (p.pblock) {
      p.pblock->cnt += p.stagecnt;
      shared_ptr_t(p.pblock);
    }
  }

  // construct
  atomic_shared_ptr():
    pblock{} {
    // pass
  }

  atomic_shared_ptr(shared_ptr_t p):
    pblock{0, std::exchange(p.pblock, nullptr)} {
    // pass
  }

  // modify
  void operator=(shared_ptr_t p_);

  shared_ptr_t exchange(shared_ptr_t p);
  bool compare_exchange_weak(shared_ptr_t& expect, const shared_ptr_t& desire);
  bool compare_exchange_weak(shared_ptr_t& expect, shared_ptr_t&& desire);
  bool compare_exchange_strong(shared_ptr_t& expect, const shared_ptr_t& desire);
  bool compare_exchange_strong(shared_ptr_t& expect, shared_ptr_t&& desire);

  // observer
  bool is_lock_free() const;
  operator shared_ptr_t() const;

private:
  mutable std::atomic<counted_ptr> pblock;

  counted_ptr copy_ptr() const {
    counted_ptr p = pblock, pp;
    do {
      if (!p.pblock) return p;
      pp = p;
      pp.stagecnt += one_stagecnt;
    }
    while (!pblock.compare_exchange_weak(p, pp));
    return pp;
  }
};

} // namespace lf

#endif // LF_ATOMIC_SHARED_PTR_HPP
