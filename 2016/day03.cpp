// https://adventofcode.com/2016/day/3

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <print>
#endif

using Triangle = std::array<u32, 3>;
using Input = Vec<Triangle>;

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let sides = aoc::split_sstream<u32>(line);
           return Triangle{sides[0], sides[1], sides[2]};
         }) |
         aoc::collect_vec<Triangle>();
}

fn is_triangle(Triangle const& triangle) -> bool {
  let[a, b, c] = triangle;
  return ((a + b) > c) && ((a + c) > b) && ((b + c) > a);
}

fn solve_case1(Input const& triangles) -> usize {
  return static_cast<usize>(stdr::count_if(triangles, is_triangle));
}

fn solve_case2(Input const& triangles) -> usize {
  AOC_ASSERT((triangles.size() % 3) == 0, "Reading 3 triangle lines at a time");
  auto count = usize{};
  for (let& rows : triangles | stdv::chunk(3)) {
    for (let column : aoc::views::indices_of(triangles[0])) {
      count += is_triangle(
          Triangle{rows[0][column], rows[1][column], rows[2][column]});
    }
  }
  return count;
}

int main() {
  std::println("Part 1");
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(3, solve_case1(example));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(993, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_case2(example));
  AOC_EXPECT_RESULT(1849, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
