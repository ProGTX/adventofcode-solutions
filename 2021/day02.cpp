// https://adventofcode.com/2021/day/2

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct command_t {
  std::string command;
  int number;
};

using commands_t = std::vector<command_t>;

commands_t parse(const std::string& filename) {
  commands_t commands;
  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto values = aoc::split_once(line, ' ');
    commands.emplace_back(std::string{values[0]},
                          aoc::to_number<int>(values[1]));
  }
  return commands;
}

int solve_part1(const commands_t& commands) {
  int horizontal = 0;
  int depth = 0;

  for (const auto& [command, number] : commands) {
    if (command == "forward") {
      horizontal += number;
    } else if (command == "down") {
      depth += number;
    } else if (command == "up") {
      depth -= number;
    }
  }

  return (horizontal * depth);
}

int solve_part2(const commands_t& commands) {
  int horizontal = 0;
  int depth = 0;
  int aim = 0;

  for (const auto& [command, number] : commands) {
    if (command == "forward") {
      horizontal += number;
      depth += (aim * number);
    } else if (command == "down") {
      aim += number;
    } else if (command == "up") {
      aim -= number;
    }
  }

  return (horizontal * depth);
}

int main() {
  std::println("Part 1");
  const auto example = parse("day02.example");
  AOC_EXPECT_RESULT(150, solve_part1(example));
  const auto input = parse("day02.input");
  AOC_EXPECT_RESULT(2120749, solve_part1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(900, solve_part2(example));
  AOC_EXPECT_RESULT(2138382217, solve_part2(input));

  AOC_RETURN_CHECK_RESULT();
}
