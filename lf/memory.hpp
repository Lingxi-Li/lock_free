#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include "utility.hpp"

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#define LF_MAKE(p, type, ...) \
  auto p = ::lf::allocate<type>(); \
  try { \
    ::lf::init(p, __VA_ARGS__); \
  } \
  catch (...) { \
    ::lf::deallocate(p); \
    throw; \
  }

namespace lf {

namespace impl {

template <typename...>
struct paren_initable: std::false_type {};

template <typename T, typename... Us>
struct paren_initable<decltype((void)T(std::declval<Us>()...)), T, Us...>:
 std::true_type {};

template <typename T, typename... Us>
inline constexpr auto paren_initable_v = paren_initable<void, T, Us...>::value;

} // namespace impl

template <typename T>
T* allocate() {
  return (T*)operator new(sizeof(T));
}

template <typename T>
T* try_allocate() noexcept {
  return (T*)operator new(sizeof(T), std::nothrow);
}

inline constexpr
struct deallocate_t {
  void operator()(void* p) const noexcept {
    operator delete(p);
  }
}
deallocate;

template <typename T, typename... Us>
void init(T*& p, Us&&... us) {
  if constexpr (impl::paren_initable_v<T, Us...>) {
    p = new(p) T(std::forward<Us>(us)...);
  }
  else {
    p = new(p) T{std::forward<Us>(us)...};
  }
}

template <typename T, typename... Us>
void init(T* const & p, Us&&... us) {
  if constexpr (impl::paren_initable_v<T, Us...>) {
    new(p) T(std::forward<Us>(us)...);
  }
  else {
    new(p) T{std::forward<Us>(us)...};
  }
}

template <typename T, typename... Us>
T emplace(Us&&... us) {
  if constexpr (impl::paren_initable_v<T, Us...>) {
    return T(std::forward<Us>(us)...);
  }
  else {
    return T{std::forward<Us>(us)...};
  }
}

template <typename T, typename... Us>
T* make(Us&&... us) {
  auto p = allocate<T>();
  try {
    init(p, std::forward<Us>(us)...);
    return p;
  }
  catch (...) {
    deallocate(p);
    throw;
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
auto make_unique(Us&&... us) {
  return unique_ptr<T>(make<T>(std::forward<Us>(us)...));
}

} // namespace lf

#endif // LF_MEMORY_HPP
