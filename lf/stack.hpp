#ifndef LF_STACK_HPP
#define LF_STACK_HPP

namespace lf {

template <typename T>
class stack {
public:
  // copy control
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;
 ~stack();

  // construct
  stack();

  // modify
  template <typename... Us>
  void emplace(Us... args);
  T pop();

  // observer
  bool is_lock_free() const;
};

} // namespace lf

#endif // LF_STACK_HPP