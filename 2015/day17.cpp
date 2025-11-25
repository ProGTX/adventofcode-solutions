// https://adventofcode.com/2015/day/17

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

template <u32 LITERS, bool COUNT_WAYS>
fn solve_case(String const& filename) -> u32 {
  let containers = [&] {
    auto array = std::array<u32, (LITERS > 25) ? 20 : 5>{};
    std::ranges::copy(aoc::views::read_numbers<u32>(filename),
                      aoc::inserter_it(array));
    return array;
  }();
  let size = containers.size();
  auto count = std::array<u32, (COUNT_WAYS ? size : 0) + 1>{};
  for (let& combination : aoc::views::binary_combinations<u32>(containers)) {
    let exact_match =
        aoc::ranges::dot_product(containers, combination) == LITERS;
    if constexpr (!COUNT_WAYS) {
      count[0] += static_cast<u32>(exact_match);
    } else if (exact_match) {
      let num_used = aoc::ranges::accumulate(combination, 0u);
      count[num_used] += 1;
    }
  }
  if constexpr (!COUNT_WAYS) {
    return count[0];
  } else {
    return *std::ranges::find_if(count, aoc::not_equal_to_value<u32>{0});
  }
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(4, (solve_case<25, false>("day17.example")));
  AOC_EXPECT_RESULT(1304, (solve_case<150, false>("day17.input")));
  std::println("Part 2");
  AOC_EXPECT_RESULT(3, (solve_case<25, true>("day17.example")));
  AOC_EXPECT_RESULT(18, (solve_case<150, true>("day17.input")));
  AOC_RETURN_CHECK_RESULT();
}
