// https://adventofcode.com/2021/day/1

#include "../common/common.h"

#include <iostream>
#include <string>

int solve_case(const std::string&) { return 0; }

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3, solve_case("day01.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(-1257, solve_case("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
