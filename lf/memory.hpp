#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include "prolog.inc"

namespace impl {

template <typename...>
struct paren_initable: std::false_type {};

template <typename T, typename... Us>
struct paren_initable<decltype((void)T(std::declval<Us>()...)), T, Us...>
 : std::true_type {};

template <typename T, typename... Us>
inline constexpr bool paren_initable_v = paren_initable<void, T, Us...>::value;

} // namespace impl

template <typename T>
T* allocate(std::size_t n = 1) {
  return n ? (T*)operator new(sizeof(T) * n) : (T*)nullptr;
}

inline
void deallocate(void* p) noexcept {
  operator delete(p);
}

template <typename T, typename... Args>
void init(T*& p, Args&&... args) {
  if constexpr (impl::paren_initable_v<T, Args...>) {
    p = new(p) T(std::forward<Args>(args)...);
  }
  else {
    p = new(p) T{std::forward<Args>(args)...};
  }
}

template <typename T, typename... Args>
void init(T*&& p, Args&&... args) {
  if constexpr (impl::paren_initable_v<T, Args...>) {
    (void)new(p) T(std::forward<Args>(args)...);
  }
  else {
    (void)new(p) T{std::forward<Args>(args)...};
  }
}

template <typename T>
void uninit(T* p) noexcept {
  p->~T();
}

#include "epilog.inc"

#endif // LF_MEMORY_HPP
