#ifndef AOC_TIMER_H
#define AOC_TIMER_H

// https://github.com/biggysmith/advent_of_code_2022/blob/master/src/util/timer.hpp

#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#include <chrono>
#include <iostream>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

class timer {
 public:
  void start() {
    start_time_ = std::chrono::system_clock::now();
    running_ = true;
  }

  void stop() {
    end_time_ = std::chrono::system_clock::now();
    running_ = false;
  }

  double milliseconds() {
    std::chrono::time_point<std::chrono::system_clock> end_time;

    if (running_) {
      end_time = std::chrono::system_clock::now();
    } else {
      end_time = end_time_;
    }

    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(
               end_time - start_time_)
        .count();
  }

  double seconds() { return milliseconds() / 1000.0; }

 private:
  std::chrono::time_point<std::chrono::system_clock> start_time_;
  std::chrono::time_point<std::chrono::system_clock> end_time_;
  bool running_ = false;
};

class scoped_timer {
 public:
  scoped_timer() { timer_.start(); }

  ~scoped_timer() {
    timer_.stop();
    std::cout << timer_.milliseconds() << "ms" << std::endl;
  }

 private:
  timer timer_;
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_TIMER_H
