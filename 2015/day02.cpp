// https://adventofcode.com/2015/day/2

#include "../common/common.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <string_view>

using dims_t = aoc::nd_point_type<int, 3>;

template <bool>
int solve_case(const std::string& filename) {
  int sum = 0;
  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto box = aoc::split<dims_t>(line, 'x');
    auto sides = dims_t{box[0] * box[1], box[0] * box[2], box[1] * box[2]};
    sum += sides.dot(dims_t{2, 2, 2}) + std::ranges::min(sides);
  }
  return sum;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT((58 + 43), solve_case<false>("day02.example"));
  AOC_EXPECT_RESULT(1606483, solve_case<false>("day02.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(5, solve_case<true>("day02.example"));
  // AOC_EXPECT_RESULT(1797, solve_case<true>("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
