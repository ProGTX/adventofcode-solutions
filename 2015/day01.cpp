// https://adventofcode.com/2015/day/1

#include "../common/common.h"

#include <fstream>
#include <iostream>
#include <string>

int solve_case(const std::string& filename) {
  int floor = 0;
  std::ifstream file{filename};
  for (char c : aoc::read_line(file)) {
    floor += static_cast<int>(c == '(') - static_cast<int>(c == ')');
  }
  return floor;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3, solve_case("day01.example"));
  AOC_EXPECT_RESULT(280, solve_case("day01.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(3, solve_case("day01.example"));
  // AOC_EXPECT_RESULT(-1257, solve_case("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
