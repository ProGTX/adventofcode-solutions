// https://adventofcode.com/2022/day/7

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

#include "../common.h"

void solve_part1(const std::string& filename) {
  int score = 0;
  readfile_op(filename, [&](std::string_view line) {});

  std::cout << filename << " -> " << score << std::endl;
}

void solve_part2(const std::string& filename) {
  int score = 0;
  readfile_op(filename, [&](std::string_view line) {});

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day07.example");
  solve_part1("day07.input");
  std::cout << "Part 2" << std::endl;
  solve_part2("day07.example");
  solve_part2("day07.input");
}
