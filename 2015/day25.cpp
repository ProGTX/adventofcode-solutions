// https://adventofcode.com/2015/day/25

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using coords_t = aoc::point_type<u32>;

auto parse(String const& filename) -> coords_t {
  let file = String{aoc::trim(aoc::read_file(filename), " \t\n\r\f\v.")};
  let[_, info] = aoc::split_once(file, "row ");
  let[row, column] = aoc::split_once<u32>(info, ", column ");
  return coords_t{
      .x = column,
      .y = row,
  };
}

struct Code {
  i64 value;
  u32 row;
  u32 column;
};

fn next(const Code code) -> Code {
  auto new_code = Code{
      .value = (code.value * 252533) % 33554393,
      .row = code.row - 1,
      .column = code.column + 1,
  };
  if (new_code.row == 0) {
    new_code.row = new_code.column;
    new_code.column = 1;
  }
  return new_code;
}

fn solve_case(const coords_t coords) -> i64 {
  let[column, row] = coords;
  auto current = Code{
      .value = 20151125,
      .row = 1,
      .column = 1,
  };
  while ((current.row != row) || (current.column != column)) {
    current = next(current);
  }
  return current.value;
}

int main() {
  std::println("Part 1");
  let example = parse("day25.example");
  AOC_EXPECT_RESULT(12231762, solve_case(example));
  let input = parse("day25.input");
  AOC_EXPECT_RESULT(2650453, solve_case(input));

  AOC_RETURN_CHECK_RESULT();
}
