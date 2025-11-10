// https://adventofcode.com/2015/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>


int solve_case(const String& filename) {
  std::ifstream file{filename};
  aoc::read_line(file);
  return -1;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  AOC_EXPECT_RESULT(156366, solve_case("day12.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  // AOC_EXPECT_RESULT(0, solve_case("day12.input"));
  AOC_RETURN_CHECK_RESULT();
}
