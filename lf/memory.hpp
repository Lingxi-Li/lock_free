#ifndef LF_MEMORY_HPP
#define LF_MEMORY_HPP

#include <memory>
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
void init(T*& p, Us&&... us) {
  p = new(p) T(std::forward<Us>(us)...);
}

template <typename T, typename... Us>
void list_init(T*& p, Us&&... us) {
  p = new(p) T{std::forward<Us>(us)...};
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

template <typename T, typename... Us>
T* list_make(Us&&... us) {
  auto p = allocate<T>();
  try {
    list_init(p, std::forward<Us>(us)...);
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

template <typename T>
struct deleter {
  void operator()(T* p) const noexcept {
    dismiss(p);
  }
};

template <typename T>
using unique_ptr = std::unique_ptr<T, deleter<T>>;

template <typename T, typename... Us>
auto make_unique(Us&&... us) {
  return unique_ptr<T>(make<T>(std::forward<Us>(us)...));
}

} // namespace lf

#endif // LF_MEMORY_HPP
