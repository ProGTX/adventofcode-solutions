// https://adventofcode.com/2021/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <print>
#include <span>
#include <string>
#include <vector>

auto parse(String const& filename) -> Vec<std::string> {
  return aoc::views::read_lines(filename) | aoc::ranges::to<Vec<std::string>>();
}

fn solve_case1(std::span<const std::string> data) -> uint64_t {
  // TODO: Implement Part 1
  return 0;
}

fn solve_case2(std::span<const std::string> data) -> uint64_t {
  // TODO: Implement Part 2
  return 0;
}

int main() {
  std::println("Part 1");
  const auto example = parse("day16.example");
  AOC_EXPECT_RESULT(0, solve_case1(example));

  const auto input = parse("day16.input");
  // AOC_EXPECT_RESULT(XXX, solve_case1(input));

  std::println("Part 2");
  // TODO: Verify example result
  // AOC_EXPECT_RESULT(XXX, solve_case2(example));
  // TODO: Verify input result
  // AOC_EXPECT_RESULT(XXX, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}