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
using blink_key_t = aoc::point_type<int_t>;
using blink_cache_t = aoc::flat_map<blink_key_t, int_t>;

constexpr int_t blink(int_t stone, int_t blink_counter,
                      blink_cache_t& blink_cache) {
  if (blink_counter == 0) {
    return 1;
  }
  const auto cache_key = blink_key_t{stone, blink_counter};
  if (auto it = blink_cache.find(cache_key); it != blink_cache.end()) {
    return it->second;
  } else {
    if (stone == 0) {
      auto num_stones = blink(1, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    } else if (int digits = aoc::num_digits(stone); (digits % 2) == 0) {
      // The stone is replaced by two stones
      const auto divider = aoc::pown(10, digits / 2);
      // The left half of the digits are engraved on the new left stone
      // The right half of the digits are engraved on the new right stone
      auto num_stones = blink(stone / divider, blink_counter - 1, blink_cache) +
                        blink(stone % divider, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    } else {
      auto num_stones = blink(stone * 2024, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    }
  }
}

template <int times>
constexpr int_t change_stones(const stones_t& stones) {
  // We need to use dynamic programming to speed up the calculation
  blink_cache_t blink_cache;
  int_t num_stones = 0;
  for (const int_t stone : stones) {
    num_stones += blink(stone, times, blink_cache);
  }
  return num_stones;
}

template <int blink_times>
int_t solve_case(const std::string& filename) {
  stones_t stones;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    stones = aoc::split<stones_t>(line, ' ');
  }

  int_t sum = 0;
  sum = change_stones<blink_times>(stones);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(55312, solve_case<25>("day11.example"));
  AOC_EXPECT_RESULT(191690, solve_case<25>("day11.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(65601038650482, solve_case<75>("day11.example"));
  AOC_EXPECT_RESULT(228651922369703, solve_case<75>("day11.input"));
  AOC_RETURN_CHECK_RESULT();
}
