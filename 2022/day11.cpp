// https://adventofcode.com/2022/day/11

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

#include "../common.h"

template <bool>
void solve_case(const std::string& filename) {
  int score = 0;

  readfile_op(filename, [&](std::string_view line) {
    auto [instruction, name, cd_to] =
        split<std::array<std::string, 3>>(std::string{line}, ' ');
  });

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day11.example");
  solve_case<false>("day11.input");
  // std::cout << "Part 2" << std::endl;
  solve_case<true>("day11.example");
  solve_case<true>("day11.input");
}
