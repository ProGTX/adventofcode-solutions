// https://adventofcode.com/2015/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <print>
#include <ranges>

template <u32 target>
fn solve_case1() -> u32 {
  static_assert(target > 10);
  for (let house : Range{2u}) {
    let presents =
        10 * aoc::ranges::accumulate(aoc::divisors<false>(house), 0u);
    if (presents >= target) {
      return house;
    }
  }
  AOC_UNREACHABLE("House should have been found in infinite loop");
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(8, solve_case1<150>());
  AOC_EXPECT_RESULT(831600, solve_case1<36'000'000>());

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3, solve_case2());
  // AOC_EXPECT_RESULT(1257, solve_case2());

  AOC_RETURN_CHECK_RESULT();
}
