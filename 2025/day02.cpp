// https://adventofcode.com/2025/day/2

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

fn parse(String const& filename) -> Vec<u64> {
  // TODO
  return {};
}

fn solve_case1(auto const& input) -> u64 {
  // TODO
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT(1227775554, solve_case1(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT(30323879646, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(4174379265, solve_case2(example));
  // AOC_EXPECT_RESULT(43872163557, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
