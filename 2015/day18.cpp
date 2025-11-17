// https://adventofcode.com/2015/day/18

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using grid_t = aoc::char_grid<>;
constexpr let light_on = '#';
constexpr let light_off = '.';

fn parse(String const& filename) -> grid_t {
  return aoc::read_char_grid(filename);
}

template <u32 STEPS, bool STUCK_CORNERS>
fn solve_case(grid_t current_lights) -> u32 {
  let side = static_cast<int>(current_lights.num_rows());
  let corners = std::array{
      point{},
      point{0, side - 1},
      point{side - 1, 0},
      point{side - 1, side - 1},
  };
  if constexpr (STUCK_CORNERS) {
    for (let corner : corners) {
      current_lights.modify(light_on, corner.y, corner.x);
    }
  }
  auto next_lights = current_lights;
  for (let _ : std::views::iota(0u, STEPS)) {
    // TODO Trying to use std::transform instead of this for loop doesn't work,
    // likely an issue with grid iterators somewhere.
    for (let[index, light] : current_lights | std::views::enumerate) {
      let pos = current_lights.position(index);
      if constexpr (STUCK_CORNERS) {
        if (std::ranges::contains(corners, pos)) {
          continue;
        }
      }
      let next = [&] {
        let num_on_neighbors = std::ranges::count(
            current_lights.all_neighbor_values(pos), light_on);
        if (light == light_on) {
          return ((num_on_neighbors == 2) || (num_on_neighbors == 3))
                     ? light_on
                     : light_off;
        } else {
          return (num_on_neighbors == 3) ? light_on : light_off;
        }
      }();
      next_lights.modify(next, pos.y, pos.x);
    }
    std::swap(current_lights, next_lights);
  }
  return std::ranges::count(current_lights, light_on);
}

int main() {
  std::println("Part 1");
  let example = parse("day18.example");
  AOC_EXPECT_RESULT(4, (solve_case<4, false>(example)));
  let input = parse("day18.input");
  AOC_EXPECT_RESULT(1061, (solve_case<100, false>(input)));
  std::println("Part 2");
  AOC_EXPECT_RESULT(17, (solve_case<5, true>(example)));
  AOC_EXPECT_RESULT(1006, (solve_case<100, true>(input)));
  AOC_RETURN_CHECK_RESULT();
}
