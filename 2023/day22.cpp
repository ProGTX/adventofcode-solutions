// https://adventofcode.com/2023/day/22

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = int;

template <bool>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  for (std::string_view line : aoc::views::read_lines(filename)) {
  }

  int_t sum = 0;
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(16, (solve_case<false>("day22.example")));
  // AOC_EXPECT_RESULT(3646, (solve_case<false>("day22.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(16, (solve_case<true>("day22.example")));
  // AOC_EXPECT_RESULT(16733044, (solve_case<true>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
