#ifndef LF_ALLOCATOR_HPP
#define LF_ALLOCATOR_HPP

#include "memory.hpp"
#include "utility.hpp"

#include <functional>

#include "prolog.inc"

template <typename T>
class allocator {
public:
  struct node {
    T val;
    std::atomic_uint32_t next;
  };

  allocator() noexcept = default;

  explicit allocator(std::uint32_t capacity):
   backup(allocate<node>(capacity)),
   head(cp_t{capacity ? 0 : null}) {
    link(capacity);
  }

  ~allocator() {
    lf::deallocate(backup);
  }

  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  void reset(std::uint32_t capacity) {
    lf::deallocate(std::exchange(backup, allocate<node>(capacity)));
    head.store({capacity ? 0 : null}, rlx);
    link(capacity);
  }

  template <typename F, typename... Args>
  void reset(std::uint32_t capacity, F&& f, Args&&... args) {
    auto newbackup = allocate<node>(capacity);
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    lf::deallocate(std::exchange(backup, newbackup));
    head.store({capacity ? 0 : null}, rlx);
    link(capacity);
  }

  std::uint32_t try_allocate() noexcept {
    cp_t newhd, hd(head.load(acq));
    do {
      if (hd.ptr == null) return null;
      auto& nod = deref(hd.ptr);
      newhd.ptr = nod.next.load(rlx);
      newhd.cnt = hd.cnt + 1;
    }
    while (!head.compare_exchange_weak(hd, newhd, rlx, acq));
    return hd.ptr;
  }

  void deallocate(std::uint32_t p) noexcept {
    auto& nod = deref(p);
    cp_t newhd{p}, hd(head.load(rlx));
    do {
      nod.next.store(hd.ptr, rlx);
      newhd.cnt = hd.cnt;
    }
    while (!head.compare_exchange_weak(hd, newhd, rel, rlx));
  }

  node& deref(std::uint32_t ptr) noexcept {
    return backup[ptr];
  }

  void del(std::uint32_t p) noexcept {
    auto& nod = deref(p);
    uninit(&nod.val);
    deallocate(p);
  }

private:
  using cp_t = counted_ptr;

  void link(std::uint32_t capacity) noexcept {
    if (!capacity) return;
    std::uint32_t i = 0, ni = 1;
    while (ni < capacity) {
      init(&backup[i].next, ni);
      i = ni++;
    }
    init(&backup[i].next, null);
  }

  node* backup{};
  std::atomic<cp_t> head{cp_t{}};
};

#include "epilog.inc"

#endif // LF_ALLOCATOR_HPP
