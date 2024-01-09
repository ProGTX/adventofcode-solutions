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

int solve_part1(const std::string& filename) {
  int horizontal = 0;
  int depth = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto values = aoc::split<std::array<std::string_view, 2>>(line, ' ');
    auto command = values[0];
    auto number = aoc::to_number<int>(values[1]);

    if (command == "forward") {
      horizontal += number;
    } else if (command == "down") {
      depth += number;
    } else if (command == "up") {
      depth -= number;
    }
  }

  std::cout << filename << " -> " << (horizontal * depth) << std::endl;
  return (horizontal * depth);
}

int solve_part2(const std::string& filename) {
  int horizontal = 0;
  int depth = 0;
  int aim = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto values = aoc::split<std::array<std::string_view, 2>>(line, ' ');
    auto command = values[0];
    auto number = aoc::to_number<int>(values[1]);

    if (command == "forward") {
      horizontal += number;
      depth += (aim * number);
    } else if (command == "down") {
      aim += number;
    } else if (command == "up") {
      aim -= number;
    }
  }

  std::cout << filename << " -> " << (horizontal * depth) << std::endl;
  return (horizontal * depth);
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(150, solve_part1("day02.example"));
  AOC_EXPECT_RESULT(2120749, solve_part1("day02.input"));
  // std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(900, solve_part2("day02.example"));
  AOC_EXPECT_RESULT(2138382217, solve_part2("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
