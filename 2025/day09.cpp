// https://adventofcode.com/2025/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Point = aoc::point_type<i64>;

fn parse(String const& filename) -> Vec<Point> {
  return aoc::views::read_lines(filename) |
         std::views::transform(
             [](str line) { return aoc::split<Point>(line, ','); }) |
         aoc::ranges::to<Vec<Point>>();
}

fn solve_case1(Vec<Point> const& points) -> usize {
  return std::ranges::max( //
      std::views::cartesian_product(points, points) |
      std::views::filter(
          [](auto&& ab) { return std::get<0>(ab) < std::get<1>(ab); }) |
      std::views::transform([](auto&& ab) {
        let & [ a, b ] = ab;
        let diff = (b - a).abs() + Point{1, 1};
        return static_cast<u64>(diff.x) * static_cast<u64>(diff.y);
      }));
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(50, solve_case1(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(4744899849, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(25272, solve_case<usize_max>(example));
  // AOC_EXPECT_RESULT(100011612, solve_case<usize_max>(input));

  AOC_RETURN_CHECK_RESULT();
}
