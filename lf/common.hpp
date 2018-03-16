#ifndef LF_COMMON_HPP
#define LF_COMMON_HPP

#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

namespace lf {

constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto cst = std::memory_order_seq_cst;

template <typename Alloc, typename... Args>
auto make(Alloc& alloc, Args&&... args) {
  auto p = alloc.allocate(1);
  using T = std::decay_t<decltype(*p)>;
  new(p) T(std::forward<Args>(args)...);
  return p;
}

template <typename Alloc, typename T>
void dismiss(Alloc& alloc, T* p) noexcept {
  p->~T();
  alloc.deallocate(p, 1);
}

// Assumes that transient ref count occupies the higher 32 bits.
// Persistent ref count occupies the lower 32 bits.
// Both work as unsigned type to wrap-around and avoid overflow.
// Combine into a single 64-bit integer, so that the two can be
// updated together in an atomic fasion.
constexpr auto one_trefcnt = (std::uint64_t)1 << 32;

// trefcnt: Transient ref count.
// To release a counted_ptr, add trefcnt to the combined count,
// and then deal with persistent ref count if necessary.
// atomic<counted_ptr> should almost always be defined as mutable.
// Also note https://stackoverflow.com/q/48947428/1348273
template <typename T>
struct counted_ptr {
  T* p;
  std::uint64_t trefcnt;
};

template <typename T>
using atomic_counted_ptr = std::atomic<counted_ptr<T>>;

template <typename T>
void hold_ptr(atomic_counted_ptr<T>& stub, counted_ptr<T>& old,
              std::memory_order success, std::memory_order fail) noexcept {
  counted_ptr<T> ne;
  do {
    ne = old;
    ne.trefcnt += one_trefcnt;
  }
  while (!stub.compare_exchange_weak(old, ne, success, fail));
  old.trefcnt = ne.trefcnt;
}

template <typename T>
bool hold_ptr_if_not_null(
  atomic_counted_ptr<T>& stub, counted_ptr<T>& old,
  std::memory_order success, std::memory_order fail) noexcept {
  counted_ptr<T> ne;
  do {
    if (!old.p) return false;
    ne = old;
    ne.trefcnt += one_trefcnt;
  }
  while (!stub.compare_exchange_weak(old, ne, success, fail));
  old.trefcnt = ne.trefcnt;
  return true;
}

template <typename T, typename Alloc = std::allocator<T>>
void unhold_ptr(T* node, bool undock, Alloc&& alloc = Alloc{}) noexcept {
  auto delta = -one_trefcnt - undock;
  if (node->cnt.fetch_add(delta, rlx) == -delta) {
    node->cnt.load(acq);
    dismiss(alloc, node);
  }
}

template <typename T>
void unhold_ptr_acq(T* node) noexcept {
  if (node->cnt.fetch_sub(one_trefcnt, rlx) == one_trefcnt) {
    node->cnt.load(acq);
    delete node;
  }
}

template <typename T, typename Alloc = std::allocator<T>>
void unhold_ptr_rel(T* node, bool undock, Alloc&& alloc = Alloc{}) noexcept {
  auto delta = -one_trefcnt - undock;
  if (node->cnt.fetch_add(delta, rel) == -delta) {
    dismiss(alloc, node);
  }
}

template <typename T>
void unhold_ptr_rel(counted_ptr<T> pc, std::uint64_t refcnt = 0) noexcept {
  auto delta = pc.trefcnt - one_trefcnt - refcnt;
  if (pc.p->cnt.fetch_add(delta, rel) == -delta) {
    delete pc.p;
  }
}

} // namespace lf

#endif // LF_COMMON_HPP
