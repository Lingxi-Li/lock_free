#include "cli.hpp"

#include <cstdint>
#include <algorithm>
#include <atomic>
#include <functional>
#include <initializer_list>
#include <random>
#include <thread>
#include <vector>

#define ERROR(...) throw ::std::runtime_error(::mkstr(__VA_ARGS__))

inline constexpr int operator""_K(unsigned long long v) noexcept { return (int)v * 1000; }
inline constexpr int operator""_M(unsigned long long v) noexcept { return (int)v * 1000'000; }
inline constexpr int operator""_B(unsigned long long v) noexcept { return (int)v * 1000'000'000; }

inline void validate_thread_cnt(unsigned thread_cnt) {
  if (thread_cnt == 0) ERROR("thread_cnt == 0");
  if (thread_cnt > std::thread::hardware_concurrency()) {
    ERROR("thread_cnt > hardware_concurrency (",
          std::thread::hardware_concurrency(), ')');
  }
}

template <typename T>
using matrix = std::vector<std::vector<T>>;

struct measure_t {
  tick::time_point begin{};
  tick::time_point end{};

  auto duration() const noexcept {
    return end - begin;
  }
};

using measure_fn = std::function<measure_t()>;

class sync_point {
public:
  void wait() const noexcept {
    while (!cont.load(std::memory_order_relaxed));
  }

  void signal() noexcept {
    cont.store(true, std::memory_order_relaxed);
  }

private:
  std::atomic_bool cont{false};
};

class simulator {
public:
  static void configure(
   std::initializer_list<measure_fn> fn_list,
   unsigned reps,
   unsigned margin) {
    fn.assign(fn_list);
    op_cnt = fn.size();
    simulator::reps = reps;
    simulator::margin = margin;
  }

  simulator():
   measures(op_cnt) {
    if (op_cnt == 0) ERROR("simulator::configure() not called.");
    auto len = reps + margin * 2;
    opseq.reserve(op_cnt * len);
    for (auto i = 0; i < op_cnt; ++i) {
      opseq.insert(opseq.end(), len, i);
      measures[i].reserve(len);
    }
    std::shuffle(opseq.begin(), opseq.end(), rnd);
  }

  void wait_and_run() {
    sync.wait();
    run();
  }

  void signal_and_run() {
    sync.signal();
    run();
  }

private:
  void run() {
    for (auto op : opseq) {
      measures[op].push_back(fn[op]());
    }
  }

  std::vector<std::uint8_t> opseq; // op_cnt * (reps + margin * 2)
  matrix<measure_t> measures;      // op_cnt * (reps + margin * 2)

  inline static std::vector<measure_fn> fn;
  inline static std::uint8_t op_cnt;
  inline static unsigned reps;
  inline static unsigned margin;

  inline static std::mt19937 rnd;
  inline static sync_point sync;
};
