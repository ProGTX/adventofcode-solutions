// https://adventofcode.com/2022/day/18

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

template <bool>
int solve_case(const std::string& filename) {
  int score = 0;

  readfile_op(filename, [&](std::string_view line) {
    auto [op, value] =
        split<std::array<std::string, 2>>(line, ' ');
  });

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day18.example");
  // solve_case<false>("day18.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day18.example");
  // solve_case<true>("day18.input");
  AOC_RETURN_CHECK_RESULT();
}
