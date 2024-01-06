// https://adventofcode.com/2023/day/15

#include "../common/common.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

template <bool>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  auto read_values = [&](std::string_view line) {};
  readfile_op(filename, read_values);

  int sum = 0;
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1320, (solve_case<false>("day15.example")));
  // AOC_EXPECT_RESULT(108857, (solve_case<false>("day15.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(64, (solve_case<true>("day15.example")));
  // AOC_EXPECT_RESULT(95273, (solve_case<true>("day15.input")));
  AOC_RETURN_CHECK_RESULT();
}
