#include <cstddef>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include <utility>

#define MAIN(...) \
  void main_(__VA_ARGS__); \
  int main(int argc, char* argv[]) { \
    impl::guarded_run(main_, argc, argv, #__VA_ARGS__); \
  } \
  void main_(__VA_ARGS__)

namespace impl {

template <typename T>
void set_arg(T& arg, char* str) {
  if (!(std::istringstream(str) >> arg)) {
    throw std::invalid_argument(
      std::string("Invalid argument: ") + str);
  }
}

inline void set_arg(std::string& arg, char* str) {
  arg = str;
}

template <typename... Us, std::size_t... Idx>
void set_args(
 std::tuple<Us...>& args, std::index_sequence<Idx...>, char* argv[]) {
  (set_arg(std::get<Idx>(args), argv[Idx]), ...);
}

template <typename... Args>
void guarded_run(void(*p)(Args...), int argc, char* argv[], const char* params) noexcept {
  --argc, ++argv;
  try {
    if (argc != sizeof...(Args)) {
      throw std::invalid_argument("Invalid number of arguments.");
    }
    std::tuple<Args...> args;
    set_args(args, std::index_sequence_for<Args...>{}, argv);
    std::apply(p, std::move(args));
  }
  catch (const std::exception& e) {
    std::cout
      << "std::exception caught with message:\n" << e.what() << '\n'
      << "Usage: (" << params << ')' << std::endl;
  }
  catch (...) {
    std::cout << "Unknown exception caught." << std::endl;
  }
}

} // namespace impl

inline auto tick() {
  return std::chrono::high_resolution_clock::now()
    .time_since_epoch().count();
}

class sync_point {
public:
  void wait() const noexcept {
    while (!cont.load(std::memory_order_relaxed));
  }

  void go() noexcept {
    cont.store(true, std::memory_order_relaxed);
  }

private:
  std::atomic_bool cont{false};
};
