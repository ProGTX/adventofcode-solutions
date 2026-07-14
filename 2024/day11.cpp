// https://adventofcode.com/2024/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <ranges>
#include <unordered_map>

using stones_t = Vec<u64>;
using blink_key_t = aoc::point_type<u64>;
using blink_cache_t = std::unordered_map<blink_key_t, u64>;

auto parse(String const& filename) -> stones_t {
  let lines = aoc::views::read_lines(filename) | aoc::collect_vec<String>();
  return aoc::split<stones_t>(lines.back(), ' ');
}

fn blink(u64 stone, u64 blink_counter, blink_cache_t& blink_cache) -> u64 {
  if (blink_counter == 0) {
    return 1;
  }
  let cache_key = blink_key_t{stone, blink_counter};
  if (let it = blink_cache.find(cache_key); it != blink_cache.end()) {
    return it->second;
  } else {
    if (stone == 0) {
      let num_stones = blink(1, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    } else if (let digits = aoc::num_digits(stone); (digits % 2) == 0) {
      // The stone is replaced by two stones
      let divider = aoc::pown(10, digits / 2);
      // The left half of the digits are engraved on the new left stone
      // The right half of the digits are engraved on the new right stone
      let num_stones = blink(stone / divider, blink_counter - 1, blink_cache) +
                       blink(stone % divider, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    } else {
      let num_stones = blink(stone * 2024, blink_counter - 1, blink_cache);
      blink_cache.try_emplace(cache_key, num_stones);
      return num_stones;
    }
  }
}

template <int times>
fn solve_case(stones_t const& stones) -> u64 {
  // We need to use dynamic programming to speed up the calculation
  auto blink_cache = blink_cache_t{};
  return aoc::ranges::accumulate(stones | stdv::transform([&](u64 stone) {
                                   return blink(stone, times, blink_cache);
                                 }),
                                 u64{0});
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(55312, solve_case<25>(example));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(191690, solve_case<25>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(65601038650482, solve_case<75>(example));
  AOC_EXPECT_RESULT(228651922369703, solve_case<75>(input));

  AOC_RETURN_CHECK_RESULT();
}
