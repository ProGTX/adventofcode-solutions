// https://adventofcode.com/2016/day/18

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

/// The first row of tiles
using Input = String;

constexpr let SAFE = '.';
constexpr let TRAP = '^';

auto parse(String const& filename) -> Input {
  return Input{aoc::trim(aoc::read_file(filename))};
}

/// The number of safe tiles in the whole room
template <usize NUM_ROWS>
fn solve_case(Input const& first_row) -> usize {
  // A safe tile pads either end, standing in for the walls of the room,
  // so that no tile is ever a special case.
  // The padding is never written to, and so stays safe for every row
  let num_tiles = first_row.size();
  auto row = String{};
  row.reserve(num_tiles + 2);
  row.push_back(SAFE);
  row.append(first_row);
  row.push_back(SAFE);

  auto num_safe = usize{};
  for (let _ : Range{0uz, NUM_ROWS}) {
    // The row becomes the next one in place:
    // the only tile a new one overwrites is the one to its left,
    // so remembering that single tile is enough to look back at
    auto left = SAFE;
    for (let index : Range{1uz, num_tiles + 1}) {
      let current = row[index];
      if (current == SAFE) {
        ++num_safe;
      }
      // A new tile is a trap exactly when its two neighbors differ,
      // which is what all four of the trap rules amount to
      row[index] = (left == row[index + 1]) ? SAFE : TRAP;
      left = current;
    }
  }
  return num_safe;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(6, solve_case<3>(Input{"..^^."}));
  let example = parse("day18.example");
  AOC_EXPECT_RESULT(38, solve_case<10>(example));
  let input = parse("day18.input");
  AOC_EXPECT_RESULT(1913, solve_case<40>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1935478, solve_case<400000>(example));
  AOC_EXPECT_RESULT(19993564, solve_case<400000>(input));

  AOC_RETURN_CHECK_RESULT();
}
