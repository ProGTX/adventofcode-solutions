// https://adventofcode.com/2024/day/8

#include "../common/common.h"

#include <array>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

template <bool>
int solve_case(const std::string& filename) {
  int sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(14, solve_case<false>("day08.example"));
  AOC_EXPECT_RESULT(502, solve_case<false>("day08.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day08.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day08.input"));
  AOC_RETURN_CHECK_RESULT();
}
