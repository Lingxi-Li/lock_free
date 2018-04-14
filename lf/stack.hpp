#ifndef LF_STACK_HPP
#define LF_STACK_HPP

#include "split_ref.hpp"

#include <optional>

namespace lf {

namespace impl {
namespace stack {

template <typename T>
struct node {
  T data;
  counted_ptr<node> next;
  std::atomic_uint64_t cnt{1};
};

} // namespace stack
} // namespace impl

template <typename T>
class stack {
  static_assert(std::is_move_constructible_v<T>);

  using node = impl::stack::node<T>;
  using cp_t = counted_ptr<node>;

public:
  stack() noexcept = default;

  template <typename BiIt>
  stack(BiIt first, BiIt last):
   head(cp_t{make_list(first, last).first}) {
  }

  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  ~stack() {
    auto p = head.load(rlx).ptr;
    while (p) {
      dismiss(std::exchange(p, p->next.ptr));
    }
  }

  template <typename... Us>
  void emplace(Us&&... us) {
    LF_MAKE(p, node,, lf::emplace<T>(std::forward<Us>(us)...));
    enlink(p, p);
  }

  template <typename BiIt>
  void bulk_push(BiIt first, BiIt last) {
    auto pr = make_list(first, last);
    enlink(pr.first, pr.second);
  }

  std::optional<T> try_pop() noexcept {
    auto orihead = head.load(rlx);
    while (true) {
      if (!hold_ptr_if_not_null(head, orihead, acq)) {
        return {};
      }
      auto p = orihead.ptr;
      if (head.compare_exchange_strong(orihead, p->next, rlx, rlx)) {
        auto val = std::make_optional(std::move(p->data));
        unhold_ptr_rel(orihead, 1);
        return val;
      }
      else {
        unhold_ptr_acq(p);
      }
    }
  }

private:
  static auto allocate_list(std::size_t n) {
    node *head = nullptr, *tail = nullptr;
    auto p = &head;
    while (n--) {
      if (!(tail = *p = try_allocate<node>())) {
        auto pp = head;
        while (pp) {
          deallocate(std::exchange(pp, pp->next.ptr));
        }
        throw std::bad_alloc{};
      }
      init_no_catch(&tail->next);
      p = &tail->next.ptr;
    }
    return std::make_pair(head, tail);
  }

  template <typename BiIt>
  static auto make_list(BiIt first, BiIt last) {
    auto n = range_extent(first, last);
    auto pr = allocate_list(n);
    auto p = pr.first;
    while (p) {
      try {
        init_no_catch(std::addressof(p->data), *--last);
        init_no_catch(&p->cnt, 1);
      }
      catch (...) {
        auto pp = pr.first;
        while (pp != p) {
          dismiss(std::exchange(pp, pp->next.ptr));
        }
        do {
          deallocate(std::exchange(pp, pp->next.ptr));
        }
        while (pp);
        throw;
      }
      p = p->next.ptr;
    }
    return pr;
  }

  void enlink(node* first, node* last) noexcept {
    cp_t neohead{first};
    last->next = head.load(rlx);
    while (!head.compare_exchange_weak(last->next, neohead, rel, rlx));
  }

  std::atomic<cp_t> head{};
};

} // namespace lf

#endif // LF_STACK_HPP
