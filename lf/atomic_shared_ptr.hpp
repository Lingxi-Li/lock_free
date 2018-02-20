#ifndef LF_ATOMIC_SHARED_PTR_HPP
#define LF_ATOMIC_SHARED_PTR_HPP

#include "shared_ptr.hpp"

#include <cstdint>
#include <atomic>

namespace lf {

namespace atomic_shared_ptr_impl {

using shared_ptr_impl::block;

// Use unsigned stagecnt to wrap-around and avoid overflow.
template <typename T>
struct counted_ptr {
  std::uint32_t stagecnt;
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
 ~atomic_shared_ptr();

  // construct
  atomic_shared_ptr() = default;
  atomic_shared_ptr(shared_ptr_t p);

  // modify
  void operator=(shared_ptr_t p);
  shared_ptr_t exchange(shared_ptr_t p);
  bool compare_exchange_weak(shared_ptr_t& expect, const shared_ptr_t& desire);
  bool compare_exchange_weak(shared_ptr_t& expect, shared_ptr_t&& desire);
  bool compare_exchange_strong(shared_ptr_t& expect, const shared_ptr_t& desire);
  bool compare_exchange_strong(shared_ptr_t& expect, shared_ptr_t&& desire);

  // observer
  bool is_lock_free() const;
  operator shared_ptr_t() const;

private:
  std::atomic<counted_ptr> pblock;
};

} // namespace lf

#endif // LF_ATOMIC_SHARED_PTR_HPP
