// https://adventofcode.com/2021/day/25

#include "../common/common.h"
#include "../common/rust.h"

#include <print>

using Seafloor = aoc::char_grid<>;

constexpr let EAST = '>';
constexpr let SOUTH = 'v';
constexpr let EMPTY = '.';

auto parse(String const& filename) -> Seafloor {
  return aoc::read_char_grid(filename);
}

fn solve_case1(Seafloor const& seafloor) -> u32 {
  auto grid = seafloor;
  u32 steps = 0;

  loop {
    ++steps;
    bool moved = false;

    // Move EAST simultaneously
    auto prev = grid;
    for (usize row = 0; row < grid.num_rows(); ++row) {
      for (usize col = 0; col < grid.num_columns(); ++col) {
        if (prev.at(row, col) == EAST) {
          let next_col = (col + 1) % grid.num_columns();
          if (prev.at(row, next_col) == EMPTY) {
            grid.modify(EMPTY, row, col);
            grid.modify(EAST, row, next_col);
            moved = true;
          }
        }
      }
    }

    // Move SOUTH simultaneously
    prev = grid;
    for (usize row = 0; row < grid.num_rows(); ++row) {
      for (usize col = 0; col < grid.num_columns(); ++col) {
        if (prev.at(row, col) == SOUTH) {
          let next_row = (row + 1) % grid.num_rows();
          if (prev.at(next_row, col) == EMPTY) {
            grid.modify(EMPTY, row, col);
            grid.modify(SOUTH, next_row, col);
            moved = true;
          }
        }
      }
    }

    if (!moved) {
      return steps;
    }
  }
}

fn solve_case2(Seafloor const& seafloor) -> u32 {
  // TODO: Implement Part 2
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day25.example");
  AOC_EXPECT_RESULT(58, solve_case1(example));
  let input = parse("day25.input");
  AOC_EXPECT_RESULT(329, solve_case1(input));

  std::println("Part 2");
  aoc::return_incomplete();
  // AOC_EXPECT_RESULT(XXX, solve_case2(example));
  // AOC_EXPECT_RESULT(XXX, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
