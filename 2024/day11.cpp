// https://adventofcode.com/2024/day/11

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::uint64_t;
using stones_t = std::vector<int_t>;

constexpr stones_t blink(const stones_t& stones) {
  auto stones_copy = stones;
  for (int copy_i = 0; const int_t stone : stones) {
    if (stone == 0) {
      stones_copy[copy_i] = 1;
    } else if (int digits = aoc::num_digits(stone); (digits % 2) == 0) {
      // The stone is replaced by two stones
      stones_copy.resize(stones_copy.size() + 1);
      const auto divider = aoc::pown(10, digits / 2);
      // The left half of the digits are engraved on the new left stone
      stones_copy[copy_i] = stone / divider;
      // The right half of the digits are engraved on the new right stone
      ++copy_i;
      stones_copy[copy_i] = stone % divider;
    } else {
      stones_copy[copy_i] = stone * 2024;
    }
    ++copy_i;
  }
  return stones_copy;
}

static_assert(std::ranges::equal(std::array{1, 2024, 1, 0, 9, 9, 2021976},
                                 blink({0, 1, 10, 99, 999})));

template <int times>
constexpr int_t change_stones(stones_t stones) {
  for (int i = 0; i < times; ++i) {
    stones = blink(stones);
  }
  return stones.size();
}

template <bool words>
int_t solve_case(const std::string& filename) {
  stones_t stones;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    stones = aoc::split<stones_t>(line, ' ');
  }

  int_t sum = 0;
  sum = change_stones<25>(stones);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(55312, solve_case<false>("day11.example"));
  AOC_EXPECT_RESULT(191690, solve_case<false>("day11.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day11.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day11.input"));
  AOC_RETURN_CHECK_RESULT();
}
