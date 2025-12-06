// https://adventofcode.com/2021/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <print>
#include <ranges>

using EnergyLevels = aoc::char_grid<>;

constexpr let usize_max = std::numeric_limits<usize>::max();

fn parse(String const& filename) -> EnergyLevels {
  return aoc::read_char_grid(filename);
}

template <usize NUM_STEPS>
fn solve_case(EnergyLevels const& energy_levels) -> usize {
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(9, solve_case<2>(example));
  let example2 = parse("day11.example2");
  AOC_EXPECT_RESULT(1656, solve_case<100>(example));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(1694, solve_case<100>(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(6, solve_case<usize_max>(example));
  // AOC_EXPECT_RESULT(195, solve_case<usize_max>(example2));
  // AOC_EXPECT_RESULT(346, solve_case<usize_max>(input));

  AOC_RETURN_CHECK_RESULT();
}
