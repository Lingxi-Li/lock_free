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
    std::atomic<node*> next;
  };

  allocator() noexcept = default;

  explicit allocator(std::size_t capacity):
   backup(allocate<node>(capacity)),
   head(cp_t{backup}) {
    link(capacity);
  }

  ~allocator() {
    lf::deallocate(backup);
  }

  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  void reset(std::size_t capacity) {
    lf::deallocate(std::exchange(backup, allocate<node>(capacity)));
    head.store({backup}, rlx);
    link(capacity);
  }

  template <typename F, typename... Args>
  void reset(std::size_t capacity, F&& f, Args&&... args) {
    auto newbackup = allocate<node>(capacity);
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    lf::deallocate(std::exchange(backup, newbackup));
    head.store({backup}, rlx);
    link(capacity);
  }

  node* try_allocate() noexcept {
    cp_t newhd, hd(head.load(acq));
    do {
      if (!hd.ptr) return nullptr;
      newhd.ptr = hd.ptr->next.load(rlx);
      newhd.cnt = hd.cnt + 1;
    }
    while (!head.compare_exchange_weak(hd, newhd, rlx, acq));
    return hd.ptr;
  }

  void deallocate(node* p) noexcept {
    cp_t newhd{p}, hd(head.load(rlx));
    do {
      p->next.store(hd.ptr, rlx);
      newhd.cnt = hd.cnt;
    }
    while (!head.compare_exchange_weak(hd, newhd, rel, rlx));
  }

  node* try_make(T&& v) noexcept {
    auto p = try_allocate();
    if (p) (void)init(&p->val, std::move(v));
    return p;
  }

  void del(node* p) noexcept {
    uninit(&p->val);
    deallocate(p);
  }

private:
  using cp_t = counted_ptr<node>;

  void link(std::size_t capacity) noexcept {
    if (!capacity) return;
    auto p = backup, last = backup + capacity - 1;
    while (p != last) {
      (void)init(&p->next, p + 1);
      ++p;
    }
    (void)init(&p->next, nullptr);
  }

  node* backup{};
  std::atomic<cp_t> head{cp_t{}};
};

#include "epilog.inc"

#endif // LF_ALLOCATOR_HPP
