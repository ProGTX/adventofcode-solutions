// https://adventofcode.com/2021/day/2

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

void solve_part1(const std::string& filename) {
  int horizontal = 0;
  int depth = 0;

  readfile_op(filename, [&](std::string_view line) {
    auto values = split<std::array<std::string, 2>>(std::string{line}, ' ');
    auto command = values[0];
    auto number = std::stoi(values[1]);

    if (command == "forward") {
      horizontal += number;
    } else if (command == "down") {
      depth += number;
    } else if (command == "up") {
      depth -= number;
    }
  });

  std::cout << filename << " -> " << (horizontal * depth) << std::endl;
}

void solve_part2(const std::string& filename) {
  int horizontal = 0;
  int depth = 0;
  int aim = 0;

  readfile_op(filename, [&](std::string_view line) {
    auto values = split<std::array<std::string, 2>>(std::string{line}, ' ');
    auto command = values[0];
    auto number = std::stoi(values[1]);

    if (command == "forward") {
      horizontal += number;
      depth += (aim * number);
    } else if (command == "down") {
      aim += number;
    } else if (command == "up") {
      aim -= number;
    }
  });

  std::cout << filename << " -> " << (horizontal * depth) << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day02.example");
  solve_part1("day02.input");
  // std::cout << "Part 2" << std::endl;
  solve_part2("day02.example");
  solve_part2("day02.input");
}
