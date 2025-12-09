// https://adventofcode.com/2025/day/8

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <set>

using Input = u32;

fn parse(String const& filename) -> Input { return 0; }

fn solve_case1(Input const& input) -> u32 { return 0; }

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT(40, solve_case1(example));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT(102816, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(25272, solve_case2(example));
  // AOC_EXPECT_RESULT(9876636978528, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
