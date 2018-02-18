#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

namespace lf {

namespace shared_ptr_impl {

// stagecnt is used to support atomic_shared_ptr.
// A unsigned type is used for stagecnt to avoid underflow, and wrap-around is OK.
// pdata lives with this.
// This expires when both refcnt and stagecnt reach zero.
// pdata is only modified at construction/destruction.
// This struct is therefore thread-safe.
template <typename T>
struct block {
  std::unique_ptr<T> pdata;
  std::atomic_uint32_t refcnt;
  std::atomic_uint32_t stagecnt;
};

} // namespace shared_ptr_impl 

template <typename T>
class shared_ptr {
public:
  // copy control
  shared_ptr(const shared_ptr& p):
    pblock(p.pblock) {
    if (pblock) ++pblock->refcnt;
  }

  shared_ptr(shared_ptr&& p) noexcept:
    pblock(std::exchange(p.pblock, nullptr)) {
    // pass
  }

 ~shared_ptr() {
    if (pblock) {
      if (--pblock->refcnt == 0 && pblock->stagecnt == 0) delete pblock;
    }
  }

  shared_ptr& operator=(shared_ptr p) noexcept {
    swap(*this, p);
    return *this;
  }

  friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
    std::swap(a.pblock, b.pblock);
  }

  // construct
  shared_ptr();
  shared_ptr(T* p);

  // modifier
  void reset(T* p = nullptr);

  // observer
  T* get() const;
  T& operator*() const;
  T* operator->() const;
  explicit operator bool() const;

private:
  using block = shared_ptr_impl::block<T>;

  block* pblock;
};

} // namespace lf
