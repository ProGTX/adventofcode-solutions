// https://adventofcode.com/2022/day/6

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

template <int window_size>
bool all_different(std::string_view window) {
  for (int i = 0; i < window_size - 1; ++i) {
    auto current = window[i];
    auto count = std::ranges::count(window | std::views::drop(i + 1), current);
    if (count > 0) {
      // Element repeats
      return false;
    }
  }
  return true;
}

template <int window_size>
void solve_case(const std::string& filename) {
  std::vector<int> positions;

  aoc::readfile_op(filename, [&](std::string_view line) {
    for (int pos = window_size - 1; pos < line.size(); ++pos) {
      int start = pos - window_size + 1;
      auto window = line.substr(start, window_size);
      if (all_different<window_size>(window)) {
        positions.push_back(pos + 1);
        return;
      }
    }
  });

  std::cout << filename << " -> " << std::endl;
  for (auto pos : positions) {
    std::cout << "  " << pos << std::endl;
  }
  std::cout << std::endl;

  if constexpr (window_size == 4) {
    if (filename.find(".example") != std::string::npos) {
      AOC_EXPECT_RESULT(7, positions[0]);
      AOC_EXPECT_RESULT(5, positions[1]);
      AOC_EXPECT_RESULT(6, positions[2]);
      AOC_EXPECT_RESULT(10, positions[3]);
      AOC_EXPECT_RESULT(11, positions[4]);
    } else {
      AOC_EXPECT_RESULT(1361, positions[0]);
    }
  } else if constexpr (window_size == 14) {
    if (filename.find(".example") != std::string::npos) {
      AOC_EXPECT_RESULT(19, positions[0]);
      AOC_EXPECT_RESULT(23, positions[1]);
      AOC_EXPECT_RESULT(23, positions[2]);
      AOC_EXPECT_RESULT(29, positions[3]);
      AOC_EXPECT_RESULT(26, positions[4]);
    } else {
      AOC_EXPECT_RESULT(3263, positions[0]);
    }
  }
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<4>("day06.example");
  solve_case<4>("day06.input");
  std::cout << "Part 2" << std::endl;
  solve_case<14>("day06.example");
  solve_case<14>("day06.input");
  AOC_RETURN_CHECK_RESULT();
}
