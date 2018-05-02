#ifndef LF_ALLOCATOR_HPP_
#define LF_ALLOCATOR_HPP_

#include "split_ref.hpp"

#include <cstdint>
#include <atomic>

#include "prolog.inc"

template <typename T>
class allocator {
public:
  struct node: T {
    std::atomic<node*> next;
  };

  allocator() noexcept = default;
  explicit allocator(std::size_t capacity);
  ~allocator();
  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  void reset(std::size_t capacity);
  node* try_allocate() noexcept;
  void deallocate(void* p) noexcept;

private:
  node* backup{};
  std::atomic_counted_ptr<node> head{};
};

#include "epilog.inc"

#endif // LF_ALLOCATOR_HPP_
