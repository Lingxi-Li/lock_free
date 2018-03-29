#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include <new>
#include <utility>

namespace lf {

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

template <typename T, typename... Us>
void construct(T* p, Us&&... us) {
  new(p) T(std::forward<Us>(us)...);
}

} // namespace lf

#endif // LF_MEMORY_HPP
