// https://adventofcode.com/2015/day/17

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <ranges>
#endif

Vec<u32> parse(String const& filename) {
  return aoc::views::read_numbers<u32>(filename) | aoc::collect_vec<u32>();
}

template <u32 LITERS, bool COUNT_WAYS>
fn solve_case(Vec<u32> const& sizes) -> u32 {
  let containers = [&] {
    auto array = std::array<u32, (LITERS > 25) ? 20 : 5>{};
    stdr::copy(sizes, aoc::inserter_it(array));
    return array;
  }();
  let size = containers.size();
  auto count = std::array<u32, (COUNT_WAYS ? size : 0) + 1>{};
  for (let& group : containers | aoc::views::binary_combinations()) {
    let sum = aoc::ranges::accumulate(group | aoc::views::deref, 0u);
    if (sum != LITERS) {
      continue;
    }
    let num_used = static_cast<u32>(group.size());
    if constexpr (!COUNT_WAYS) {
      count[0] += 1;
    } else {
      count[num_used] += 1;
    }
  }
  if constexpr (!COUNT_WAYS) {
    return count[0];
  } else {
    return *stdr::find_if(count, aoc::not_equal_to_value<u32>{0});
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day17.example");
  AOC_EXPECT_RESULT(4, (solve_case<25, false>(example)));
  let input = parse("day17.input");
  AOC_EXPECT_RESULT(1304, (solve_case<150, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3, (solve_case<25, true>(example)));
  AOC_EXPECT_RESULT(18, (solve_case<150, true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
