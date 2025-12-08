// https://adventofcode.com/2025/day/7

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <set>

using Manifolds = aoc::char_grid<>;
using Input = std::pair<Manifolds, point>;
constexpr let wall = '#';
constexpr let empty = '.';
constexpr let splitter = '^';

fn parse(String const& filename) -> Input {
  auto [manifolds, config] = aoc::read_char_grid(
      filename, {.padding = wall, .start_char = 'S', .end_char = {}});
  return {std::move(manifolds), *config.start_pos};
}

fn solve_case1(Input const& input) -> u32 {
  let & [ manifolds, start_pos ] = input;
  auto beams = std::set<point>{};
  beams.insert(start_pos);
  auto num_splits = 0u;
  while (!beams.empty()) {
    auto next_beams = std::set<point>{};
    for (auto beam_pos : beams) {
      beam_pos += aoc::get_diff(aoc::south);
      let value = manifolds.at(beam_pos.y, beam_pos.x);
      switch (value) {
        case empty:
          next_beams.insert(beam_pos);
          break;
        case splitter:
          num_splits += 1;
          next_beams.insert(beam_pos + aoc::get_diff(aoc::west));
          next_beams.insert(beam_pos + aoc::get_diff(aoc::east));
          break;
        case wall:
          // Do nothing
          break;
        default:
          AOC_UNREACHABLE("Invalid value in the manifold");
      }
    }
    beams = std::move(next_beams);
  }
  return num_splits;
}

int main() {
  std::println("Part 1");
  let example = parse("day07.example");
  AOC_EXPECT_RESULT(21, solve_case1(example));
  let input = parse("day07.input");
  AOC_EXPECT_RESULT(1560, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(40, solve_case2(example));
  // AOC_EXPECT_RESULT(9876636978528, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
