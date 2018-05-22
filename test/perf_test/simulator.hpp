#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "utility.hpp"

#include <cstdint>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

struct tp_pr {
  tick::time_point begin{};
  tick::time_point end{};

  auto duration() const noexcept {
    return end - begin;
  }
};

using measure_fn = std::function<tp_pr()>;

class simulator {
public:
  static std::size_t estimate_size(
   std::size_t thread_cnt, std::size_t op_cnt, std::size_t reps, std::size_t margin) noexcept {
    auto len = reps + margin * 2;
    auto opseq_sz = sizeof(std::uint8_t) * op_cnt * len;
    auto measures_sz = sizeof(tp_pr) * op_cnt * len;
    auto data_sz = opseq_sz + measures_sz;
    return data_sz * thread_cnt;
  }

  static void configure(
   std::size_t thread_cnt,
   std::size_t reps,
   std::size_t margin,
   std::vector<measure_fn> fn_list) {
    std::cout << "Configuring..." << std::endl;
    validate_thread_cnt(thread_cnt);
    simulator::thread_cnt = thread_cnt;
    fn = std::move(fn_list);
    op_cnt = fn.size();
    simulator::reps = reps;
    simulator::margin = margin;
  }

  static void kickoff() {
    std::cout << "Running..." << std::endl;
    data.resize(thread_cnt);
    threads.reserve(thread_cnt - 1);
    for (std::size_t i = 0; i < thread_cnt - 1; ++i) {
      threads.emplace_back(&per_thread_data::wait_and_run, &data[i]);
    }
    data.back().signal_and_run();
    for_each_element(&std::thread::join, threads.begin(), threads.end());
    validate_result();
  }

  static void write_result(const std::string& name, char delim) {
    std::cout << "Writing result to file " + name + "..." << std::endl;
    std::ofstream file(name);
    file.exceptions(file.badbit | file.failbit);
    for (std::size_t op = 0; op < op_cnt; ++op) {
      data[0].write_result(file, op, delim);
      for (std::size_t i = 1; i < thread_cnt; ++i) {
        data[i].write_result(file << delim, op, delim);
      }
      file << std::endl;
    }
  }

private:
  static void validate_result() {
    std::cout << "Validating result..." << std::endl;
    auto [begin, end] = data[0].stable_timespan();
    for (std::size_t i = 1; i < thread_cnt; ++i) {
      auto [cur_begin, cur_end] = data[i].stable_timespan();
      begin = std::max(begin, cur_begin);
      end = std::min(end, cur_end);
    }
    for (auto& td : data) {
      td.validate_result(begin, end);
    }
  }

  class per_thread_data {
  public:
    per_thread_data():
     measures(op_cnt) {
      auto len = reps + margin * 2;
      opseq.reserve(op_cnt * len);
      for (std::size_t i = 0; i < op_cnt; ++i) {
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
      sync.wait_and_signal(thread_cnt - 1);
      run();
    }

    tp_pr stable_timespan() const noexcept {
      auto begin = measures[0].front().end;
      auto end = measures[0].back().begin;
      for (std::size_t i = 1; i < op_cnt; ++i) {
        begin = std::min(begin, measures[i].front().end);
        end = std::max(end, measures[i].back().begin);
      }
      return {begin, end};
    }

    void validate_result(tick::time_point begin, tick::time_point end) const {
      for (auto& row : measures) {
        if (row[margin].begin < begin || row[margin + reps - 1].end > end) {
          ERROR("Result validation failed.");
        }
      }
    }

    void write_result(std::ostream& os, std::size_t op, char delim) const {
      os << measures[op][margin].duration().count();
      for (auto i = margin + 1; i < margin + reps; ++i) {
        os << delim << measures[op][i].duration().count();
      }
    }

  private:
    void run() {
      for (auto op : opseq) {
        measures[op].push_back(fn[op]());
      }
    }

    std::vector<std::uint8_t> opseq; // op_cnt * (reps + margin * 2)
    matrix<tp_pr> measures;          // op_cnt * (reps + margin * 2)
  };

  inline static std::size_t thread_cnt;
  inline static std::vector<measure_fn> fn;
  inline static std::size_t op_cnt;
  inline static std::size_t reps;
  inline static std::size_t margin;

  inline static std::vector<per_thread_data> data;
  inline static std::vector<std::thread> threads;

  inline static std::mt19937_64 rnd{};
  inline static sync_point sync;
};

#endif // SIMULATOR_HPP
