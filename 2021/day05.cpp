// https://adventofcode.com/2021/day/5

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

int solve_part1(const std::string& filename) {
  int score = 0;
  readfile_op(filename, [&](std::string_view) {});

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int solve_part2(const std::string& filename) {
  int score = 0;
  readfile_op(filename, [&](std::string_view) {});

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day05.example");
  solve_part1("day05.input");
  std::cout << "Part 2" << std::endl;
  solve_part2("day05.example");
  solve_part2("day05.input");
  AOC_RETURN_CHECK_RESULT();
}
