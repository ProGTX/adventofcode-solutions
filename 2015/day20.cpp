// https://adventofcode.com/2015/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cmath>
#include <print>
#include <ranges>

template <u32 target>
fn solve_case1() -> u32 {
  static_assert(target > 10);
  auto divisors = Vec<u32>{};
  for (let house : Range{2u}) {
    divisors.clear();
    aoc::divisors<false>(divisors, house);
    let presents = 10 * aoc::ranges::accumulate(divisors, 0u);
    if (presents >= target) {
      return house;
    }
  }
  AOC_UNREACHABLE("House should have been found in infinite loop");
}

fn num_presents(u32 house_id) -> u32 {
  return 11 * //
         aoc::ranges::accumulate(
             aoc::divisors(house_id) | stdv::filter([house_id](u32 divisor) {
               return house_id <= 50 * divisor;
             }),
             0u);
}

template <u32 target>
fn solve_case2() -> u32 {
  for (let house : Range{2u}) {
    if (num_presents(house) >= target) {
      return house;
    }
  }
  AOC_UNREACHABLE("House should have been found in infinite loop");
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(8, solve_case1<150>());
  AOC_EXPECT_RESULT(831600, solve_case1<36'000'000>());

  std::println("Part 2");
  AOC_EXPECT_RESULT(8, solve_case2<150>());
  AOC_EXPECT_RESULT(884520, solve_case2<36'000'000>());

  AOC_RETURN_CHECK_RESULT();
}
