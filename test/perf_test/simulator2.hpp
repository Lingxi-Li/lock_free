#ifndef SIMULATOR2_HPP
#define SIMULATOR2_HPP

#include "utility.hpp"

#include <cstdint>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

class simulator2 {
public:
  using fn_t = void(*)();

  static void configure(
   std::uint8_t thread_cnt,
   std::chrono::minutes expected_duration,
   std::vector<fn_t> fn) {
    simulator2::thread_cnt = thread_cnt;
    simulator2::expected_duration = expected_duration;
    sync.expected_cnt = thread_cnt;
    op_cnt = fn.size();
    simulator2::fn = std::move(fn);
    per_thread.resize(thread_cnt);
  }

  static void kickoff() {
    thread.reserve(thread_cnt);
    for (std::uint8_t i = 0; i < thread_cnt; ++i) {
      thread.emplace_back(&per_thread_t::sync_and_run, &per_thread[i]);
    }
    std::this_thread::sleep_for(expected_duration);
    stop.store(true, std::memory_order_release);
    for_each_element(&std::thread::join, thread.begin(), thread.end());
  }

  static void print_results() {
    auto [cnt, di, da] = per_thread[0].results();
    for (std::uint8_t i = 1; i < thread_cnt; ++i) {
      auto [cur_cnt, cur_di, cur_da] = per_thread[i].results();
      cnt += cur_cnt;
      di = std::min(di, cur_di);
      da = std::max(da, cur_da);
    }
    auto dur = (da - di).count();
    std::cout << "count: " << cnt       << '\n'
              << "ticks: " << dur       << '\n'
              << "ratio: " << cnt / dur << std::endl;
  }

private:
  class per_thread_t {
  public:
    per_thread_t() {
      for (std::uint8_t i = 0; i < op_cnt; ++i) {
        op_seq.insert(op_seq.end(), 1_K, i);
      }
      std::shuffle(op_seq.begin(), op_seq.end(), rnd);
    }

    void sync_and_run() {
      sync();
      di = tick::now();
      run();
      da = tick::now();
    }

    std::tuple<std::uint64_t, tick::time_point, tick::time_point>
    results() const noexcept {
      return {cnt, di, da};
    }

  private:
    void run() {
      while (!stop.load(std::memory_order_acquire)) {
        auto op = op_seq[++cnt % op_seq.size()];
        fn[op]();
      }
    }

    std::vector<std::uint8_t> op_seq;

    std::uint64_t cnt{};
    tick::time_point di, da;
  };

  inline static std::uint8_t thread_cnt;
  inline static std::chrono::minutes expected_duration;
  inline static sync_point sync;
  inline static std::uint8_t op_cnt;
  inline static std::vector<fn_t> fn;
  inline static std::vector<per_thread_t> per_thread;

  inline static std::vector<std::thread> thread;

  inline static std::atomic_bool stop{false};
  inline static std::mt19937 rnd{};
};

#endif // SIMULATOR2_HPP
