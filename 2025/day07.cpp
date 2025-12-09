// https://adventofcode.com/2025/day/7

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Manifolds = aoc::char_grid<>;
using Input = std::pair<Manifolds, point>;
constexpr let empty = '.';
constexpr let splitter = '^';

fn parse(String const& filename) -> Input {
  auto [manifolds, config] = aoc::read_char_grid(
      filename, {.padding = {}, .start_char = 'S', .end_char = {}});
  return {std::move(manifolds), *config.start_pos};
}

template <bool quantum>
fn solve_case(Input const& input) -> u64 {
  let & [ manifolds, start_pos ] = input;
  auto num_splits = 0;
  auto row_timelines = Vec(manifolds.num_columns(), u64{});
  row_timelines[start_pos.x] = 1;
  for (let row :
       Range{static_cast<usize>(start_pos.y + 1), manifolds.num_rows()}) {
    auto next_timelines = Vec(manifolds.num_columns(), u64{});
    for (let[column, timelines] : row_timelines | std::views::enumerate) {
      let value = manifolds.at(row, column);
      switch (value) {
        case empty:
          next_timelines[column] += timelines;
          break;
        case splitter: {
          num_splits += static_cast<u64>(timelines > 0);
          next_timelines[column - 1] += timelines;
          next_timelines[column + 1] += timelines;
        } break;
        default:
          AOC_UNREACHABLE("Invalid value in the manifold");
      };
    }
    row_timelines = std::move(next_timelines);
  }
  if constexpr (!quantum) {
    return num_splits;
  } else {
    return aoc::ranges::accumulate(row_timelines, u64{});
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day07.example");
  AOC_EXPECT_RESULT(21, solve_case<false>(example));
  let example2 = parse("day07.example2");
  AOC_EXPECT_RESULT(16, solve_case<false>(example2));
  let input = parse("day07.input");
  AOC_EXPECT_RESULT(1560, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(40, solve_case<true>(example));
  AOC_EXPECT_RESULT(26, solve_case<true>(example2));
  AOC_EXPECT_RESULT(25592971184998, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
