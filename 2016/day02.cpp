// https://adventofcode.com/2016/day/2

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

auto parse(String const& filename) -> Vec<i32> { return {}; }

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  // AOC_EXPECT_RESULT(1985, solve_case1(example));
  let input = parse("day02.input");
  // AOC_EXPECT_RESULT(1337, solve_case1(input));
  aoc::return_incomplete();

  std::println("Part 2");
  // AOC_EXPECT_RESULT(1337, solve_case2(example));
  // AOC_EXPECT_RESULT(1337, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
