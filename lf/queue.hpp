#ifndef LF_QUEUE_HPP
#define LF_QUEUE_HPP

#include "common.hpp"

namespace lf {

template <typename T>
class queue {
public:
  // copy control
  queue(const queue&) = delete;
  queue& operator=(const queue&) = delete;
 ~queue();

  // construct
  queue();

  // modifier
  bool try_pop(T& v);
  template <typename... Us>
  void emplace(Us&&... args);
};

} // namespace lf

#endif // LF_QUEUE_HPP