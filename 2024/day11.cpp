// https://adventofcode.com/2024/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <ranges>

using stones_t = Vec<u64>;
using blink_state_t = aoc::point_type<u64>; // x: stone, y: blink_counter

auto parse(String const& filename) -> stones_t {
  let lines = aoc::views::read_lines(filename) | aoc::collect_vec<String>();
  return aoc::split<stones_t>(lines.back(), ' ');
}

fn blink_neighbors(blink_state_t const& state)
    -> aoc::static_vector<blink_state_t, 2> {
  let[stone, blink_counter] = state;
  auto neighbors = aoc::static_vector<blink_state_t, 2>{};
  {
    if (stone == 0) {
      neighbors.emplace_back(u64{1}, blink_counter - 1);
    } else if (let digits = aoc::num_digits(stone); (digits % 2) == 0) {
      // The stone is replaced by two stones
      let divider = aoc::pown(10, digits / 2);
      // The left half of the digits are engraved on the new left stone
      // The right half of the digits are engraved on the new right stone
      neighbors.emplace_back(stone / divider, blink_counter - 1);
      neighbors.emplace_back(stone % divider, blink_counter - 1);
    } else {
      neighbors.emplace_back(stone * 2024, blink_counter - 1);
    }
  }
  return neighbors;
}

template <int times>
fn solve_case(stones_t const& stones) -> u64 {
  // We need to use dynamic programming to speed up the calculation
  return aoc::ranges::accumulate(
      stones | stdv::transform([](u64 stone) {
        let start = blink_state_t{stone, static_cast<u64>(times)};
        let num_stones = aoc::dfs(
            start, [](blink_state_t const& state) { return state.y == 0; },
            [](blink_state_t const& state) {
              return blink_neighbors(state) |
                     aoc::dijkstra_uniform_neighbors_view();
            });
        return num_stones.at(start);
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
