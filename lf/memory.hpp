#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include "utility.hpp"

#include <cstdint>
#include <memory>
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

#define LF_MAKE(p, T, ...) \
  auto p = ::lf::allocate<T>(); \
  ::lf::init(p __VA_ARGS__)

#define LF_MAKE_UNIQUE(p, T, ...) \
  auto LF_UNI_NAME(p) = ::lf::allocate<T>(); \
  auto p = ::lf::init_unique(LF_UNI_NAME(p) __VA_ARGS__)

template <typename T>
T* allocate(std::size_t n = 1) {
  return (T*)operator new(sizeof(T) * n);
}

template <typename T>
T* try_allocate(std::size_t n = 1) noexcept {
  return (T*)operator new(sizeof(T) * n, std::nothrow);
}

inline constexpr
struct deallocate_t {
  void operator()(void* p) const noexcept {
    operator delete(p);
  }
}
deallocate;

inline constexpr
struct init_no_catch_t {
  template <typename T, typename... Us>
  void operator()(T*& p, Us&&... us) const {
    if constexpr (impl::paren_initable_v<T, Us...>) {
      p = new(p) T(std::forward<Us>(us)...);
    }
    else {
      p = new(p) T{std::forward<Us>(us)...};
    }
  }
  template <typename T, typename... Us>
  void operator()(T* const & p, Us&&... us) const {
    if constexpr (impl::paren_initable_v<T, Us...>) {
      new(p) T(std::forward<Us>(us)...);
    }
    else {
      new(p) T{std::forward<Us>(us)...};
    }
  }
}
init_no_catch;

inline constexpr
struct init_t {
  template <typename P, typename... Us>
  void operator()(P&& p, Us&&... us) const {
    try {
      init_no_catch(std::forward<P>(p), std::forward<Us>(us)...);
    }
    catch (...) {
      deallocate(p);
      throw;
    }
  }
}
init;

template <typename T, typename... Us>
T emplace(Us&&... us) {
  if constexpr (impl::paren_initable_v<T, Us...>) {
    return T(std::forward<Us>(us)...);
  }
  else {
    return T{std::forward<Us>(us)...};
  }
}

inline constexpr
struct dismiss_t {
  template <typename T>
  void operator()(T* p) const noexcept {
    p->~T();
    deallocate(p);
  }
}
dismiss;

template <typename T>
using unique_ptr = std::unique_ptr<T, dismiss_t>;

template <typename T, typename... Us>
auto init_unique(T* p, Us&&... us) {
  init(p, std::forward<Us>(us)...);
  return unique_ptr<T>(p);
}

#include "epilog.inc"

#endif // LF_MEMORY_HPP
