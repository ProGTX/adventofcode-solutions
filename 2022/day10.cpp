// https://adventofcode.com/2022/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct instruction {
  enum type {
    noop,
    addx,
  };

  constexpr instruction() : instruction{noop, 1, 0} {}

  static constexpr instruction make_addx(int value) { return {addx, 2, value}; }

  constexpr void clear() { *this = instruction{}; }

  constexpr bool done() const { return m_cycles_left <= 0; }

  constexpr int tick(int register_value) {
    if (this->done()) {
      return register_value;
    }
    --m_cycles_left;
    if (m_cycles_left > 0) {
      return register_value;
    }
    switch (m_type) {
      case addx:
        return register_value + m_value;
      default:
        return register_value;
    }
  }

  constexpr std::string_view get_type_str() const {
    switch (m_type) {
      case noop:
        return "noop";
      case addx:
        return "addx";
      default:
        return "";
    }
  }

  constexpr int get_duration() const { return m_duration; }

  friend std::ostream& operator<<(std::ostream& out, const instruction& op) {
    out << "{" << op.get_type_str() << "," << op.m_cycles_left << "("
        << op.m_duration << ")," << op.m_value << "}";
    return out;
  }

 protected:
  constexpr instruction(type op_type, int duration, int value)
      : m_type{op_type},
        m_duration{duration},
        m_cycles_left{duration},
        m_value{value} {}

 private:
  type m_type;
  int m_duration;
  int m_cycles_left;
  int m_value;
};

template <bool insert_noop>
int solve_case(const std::string& filename) {
  static constexpr size_t width = 40;
  static constexpr size_t height = 6;

  using crt = aoc::array_grid<char, width, height>;
  crt monitor;
  typename crt::row_t row;
  for (int i = 0; i < width; ++i) {
    row[i] = '.';
  }
  for (int i = 0; i < height; ++i) {
    monitor.add_row(row);
  }

  int signal_strength = 0;

  int register_X = 1;
  int pc = 1;

  constexpr int pipeline_length = 2;
  std::array<instruction, pipeline_length> pipeline;
  int row_index = -1;

  const auto draw = [&] {
    int column_index = (pc - 1) % width;
    row_index = (column_index == 0) ? ((row_index + 1) % height) : row_index;
    auto pixel = std::invoke([&] {
      if (std::abs(column_index - register_X) <= 1) {
        return '#';
      }
      return '.';
    });
    monitor.modify(pixel, row_index, column_index);
  };

  const auto shift_pipeline = [&]() {
    // During cycle
    if ((pc == 20) || (((pc - 20) % 40) == 0)) {
      signal_strength += (pc * register_X);
    }
    draw();
    for (auto& op : pipeline) {
      register_X = op.tick(register_X);
    }
    // After cycle
    ++pc;
    std::shift_right(std::begin(pipeline), std::end(pipeline), 1);
    // Always insert a noop at the beginning, should be overriden later
    pipeline.front().clear();
  };

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [op, value] = aoc::split_once(line, ' ');

    // Place new instruction into the pipeline
    if (op == "noop") {
      pipeline.front().clear();
    } else if (op == "addx") {
      pipeline.front() = instruction::make_addx(aoc::to_number<int>(value));
    } else {
      throw std::runtime_error("Invalid instruction " + std::string{op});
    }

    if constexpr (insert_noop) {
      // Instruction needs to block until it's finished
      auto number_noops = pipeline.front().get_duration() - 1;
      for (int i = 0; i < number_noops; ++i) {
        shift_pipeline();
        pipeline.front().clear();
      }
    }

    shift_pipeline();
  }

  monitor.print_all();

  std::cout << filename << " -> " << signal_strength << std::endl;
  return signal_strength;
}

int main() {
  AOC_EXPECT_RESULT(13140, solve_case<true>("day10.example"));
  AOC_EXPECT_RESULT(11820, solve_case<true>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
