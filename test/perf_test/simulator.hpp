#ifndef SIMULATOR_HPP_
#define SIMULATOR_HPP_

#include "utility.hpp"

#include <cstdint>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <random>
#include <thread>
#include <vector>

struct measure_t {
  tick::time_point begin{};
  tick::time_point end{};

  auto duration() const noexcept {
    return end - begin;
  }
};

using measure_fn = std::function<measure_t()>;

class simulator {
public:
  static void configure(
   unsigned thread_cnt,
   unsigned reps,
   unsigned margin,
   std::initializer_list<measure_fn> fn_list) {
    validate_thread_cnt(thread_cnt);
    simulator::thread_cnt = thread_cnt;
    fn.assign(fn_list);
    op_cnt = fn.size();
    simulator::reps = reps;
    simulator::margin = margin;
  }

  static void kickoff() {
    if (thread_cnt == 0) ERROR("simulator not configured yet.");
    data.resize(thread_cnt);
    runner.reserve(thread_cnt - 1);
    for (auto i = 0u; i < thread_cnt - 1; ++i) {
      runner.emplace_back(&simulator::wait_and_run, &data[i]);
    }
    data.back().signal_and_run();
    for_each_element(&std::thread::join, runner.begin(), runner.end());
  }

  simulator():
   measures(op_cnt) {
    auto len = reps + margin * 2;
    opseq.reserve(op_cnt * len);
    for (auto i = 0u; i < op_cnt; ++i) {
      opseq.insert(opseq.end(), len, i);
      measures[i].reserve(len);
    }
    std::shuffle(opseq.begin(), opseq.end(), rnd);
  }

private:
  void wait_and_run() {
    sync.wait();
    run();
  }

  void signal_and_run() {
    sync.signal(thread_cnt - 1);
    run();
  }

  void run() {
    for (auto op : opseq) {
      measures[op].push_back(fn[op]());
    }
  }

  std::vector<std::uint8_t> opseq; // op_cnt * (reps + margin * 2)
  matrix<measure_t> measures;      // op_cnt * (reps + margin * 2)

  inline static unsigned thread_cnt;
  inline static std::vector<measure_fn> fn;
  inline static unsigned op_cnt;
  inline static unsigned reps;
  inline static unsigned margin;

  inline static std::vector<simulator> data;
  inline static std::vector<std::thread> runner;

  inline static std::mt19937 rnd{std::mt19937::default_seed};
  inline static sync_point sync;
};

#endif // SIMULATOR_HPP_
