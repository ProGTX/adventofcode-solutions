// https://adventofcode.com/2015/day/1

#include "../common/common.h"

#include <fstream>
#include <iostream>
#include <print>
#include <string>

template <bool check_position>
int solve_case(const std::string& filename) {
  int floor = 0;
  [[maybe_unused]] int position = 0;
  std::ifstream file{filename};
  for (char c : aoc::read_line(file)) {
    floor += static_cast<int>(c == '(') - static_cast<int>(c == ')');
    if constexpr (check_position) {
      ++position;
      if (floor == -1) {
        break;
      }
    }
  }
  return check_position ? position : floor;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(-1, solve_case<false>("day01.example"));
  AOC_EXPECT_RESULT(280, solve_case<false>("day01.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(5, solve_case<true>("day01.example"));
  AOC_EXPECT_RESULT(1797, solve_case<true>("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
