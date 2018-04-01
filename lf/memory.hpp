#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace lf {

namespace impl {

template <typename T, typename... Us>
auto init(char, T*& p, Us&&... us)
-> decltype((void)new(p) T(std::forward<Us>(us)...)) {
  p = new(p) T(std::forward<Us>(us)...);
}

template <typename T, typename... Us>
void init(int, T*& p, Us&&... us) {
  p = new(p) T{std::forward<Us>(us)...};
}

} // namespace impl

template <typename T>
T* allocate() {
  return (T*)operator new(sizeof(T));
}

template <typename T>
T* try_allocate() noexcept {
  return (T*)operator new(sizeof(T), std::nothrow);
}

inline
void deallocate(void* p) noexcept {
  operator delete(p);
}

const struct deallocator_t {
  void operator()(void* p) const noexcept {
    deallocate(p);
  }
} deallocator;

// template <typename T, typename... Us>
// void init(T*& p, Us&&... us) {
//   if constexpr (std::is_aggregate_v<T>) {
//     p = new(p) T{std::forward<Us>(us)...};
//   }
//   else {
//     p = new(p) T(std::forward<Us>(us)...);
//   }
// }

template <typename T, typename... Us>
void init(T*& p, Us&&... us) {
  impl::init(char{}, p, std::forward<Us>(us)...);
}

template <typename T, typename... Us>
T* make(Us&&... us) {
  auto p = allocate<T>();
  try {
    init(p, std::forward<Us>(us)...);
  }
  catch (...) {
    deallocate(p);
    throw;
  }
  return p;
}

template <typename T>
void dismiss(T* p) noexcept {
  p->~T();
  deallocate(p);
}

const struct deleter_t {
  template <typename T>
  void operator()(T* p) const noexcept {
    dismiss(p);
  }
} deleter;

template <typename T>
using unique_ptr = std::unique_ptr<T, deleter_t>;

template <typename T, typename... Us>
auto make_unique(Us&&... us) {
  return unique_ptr<T>(make<T>(std::forward<Us>(us)...));
}

} // namespace lf

#endif // LF_MEMORY_HPP
