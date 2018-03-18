#ifndef LF_QUEUE_HPP
#define LF_QUEUE_HPP

#include "common.hpp"

#include <atomic>
#include <memory>
#include <utility>

namespace lf {

namespace queue_impl {

template <typename T>
struct node {
 ~node() {
    // delete nullptr is OK
    delete data.load(rlx);
  }

  std::atomic<T*> data{};
  std::atomic<node*> next{};
  std::atomic_uint64_t cnt{2};
};

} // namespace queue_impl

template <typename T>
class queue {
  using node = queue_impl::node<T>;
  using counted_ptr = lf::counted_ptr<node>;

public:
  // copy control
  queue(const queue&) = delete;
  queue& operator=(const queue&) = delete;

 ~queue() {
    auto p = head.load(rlx).p;
    while (p) {
      delete std::exchange(p, p->next.load(rlx));
    }
  }

  // construct
  queue():
    head({new node{}}),
    tail(head.load(rlx)) {
  }

  // modifier
  bool try_pop(T& v) noexcept {
    auto oldhead = head.load(rlx);
    while (true) {
      hold_ptr(head, oldhead, acq, rlx);
      auto p = oldhead.p;
      if (p == tail.load(acq).p) {
        unhold_ptr_acq(p);
        return false;
      }
      counted_ptr newhead{p->next.load(rlx)};
      if (head.compare_exchange_strong(oldhead, newhead, rel, rlx)) {
        v = std::move(*p->data.load(rlx));
        unhold_ptr_rel(oldhead, 1);
        return true;
      }
      unhold_ptr_acq(p);
    }
  }

  template <typename... Us>
  void emplace(Us&&... args) {
    auto dat = std::make_unique<T>(std::forward<Us>(args)...);
    auto nod = std::make_unique<node>();
    auto oritail = tail.load(rlx);
    do {
      if (!nod) nod = std::make_unique<node>();
      hold_ptr(tail, oritail, rlx, rlx);
      auto p = oritail.p;
      T* oridat = nullptr;
      if (p->data.compare_exchange_strong(oridat, dat.get(), rlx, rlx)) {
        dat.release();
      }
      node *orinod = nullptr, *neotailnod = nod.get();
      if (p->next.compare_exchange_strong(orinod, neotailnod, rlx, rlx)) {
        nod.release();
      }
      else {
        neotailnod = orinod;
      }
      counted_ptr neotail{neotailnod};
      if (tail.compare_exchange_strong(oritail, neotail, rel, rlx)) {
        unhold_ptr_acq(oritail);
        oritail = neotail;
      }
      else {
        unhold_ptr_acq(p);
      }
    }
    while (dat);
  }

private:
  std::atomic<counted_ptr> head;
  std::atomic<counted_ptr> tail;
};

} // namespace lf

#endif // LF_QUEUE_HPP
