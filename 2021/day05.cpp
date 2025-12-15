// https://adventofcode.com/2021/day/5

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Point = aoc::point_type<i32>;

using Segments = Vec<std::pair<Point, Point>>;

auto parse(String const& filename) -> std::pair<Segments, Point> {
  auto dimensions = Point{};
  auto segments = aoc::views::read_lines(filename) |
                  std::views::transform([&](str line) {
                    let[begin, end] = aoc::split_once(line, " -> ");
                    let segment = std::pair{aoc::split<Point>(begin, ","),
                                            aoc::split<Point>(end, ",")};
                    dimensions =
                        dimensions.max(segment.first).max(segment.second);
                    return segment;
                  }) |
                  aoc::ranges::to<Segments>();
  return {std::move(segments), dimensions + Point{1, 1}};
}

template <bool DIAGONAL>
fn solve_case(let& segment_pair) -> u32 {
  let & [ segments, dimensions ] = segment_pair;
  auto intersections = aoc::grid<u32>{0, static_cast<usize>(dimensions.y),
                                      static_cast<usize>(dimensions.x)};
  for (let& [ begin, end ] : segments) {
    if constexpr (!DIAGONAL) {
      if ((begin.x != end.x) && (begin.y != end.y)) {
        continue;
      }
    }
    let direction = (end - begin).normal();
    auto pos = begin;
    while (pos != end) {
      auto& current = intersections.at(pos.y, pos.x);
      current += 1;
      pos += direction;
    }
    // Once more for the end point
    auto& current = intersections.at(pos.y, pos.x);
    current += 1;
  }
  return std::ranges::count_if(intersections, [](u32 v) { return v > 1; });
}

int main() {
  std::println("Part 1");
  let example = parse("day05.example");
  AOC_EXPECT_RESULT(5, solve_case<false>(example));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT(6461, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(12, solve_case<true>(example));
  AOC_EXPECT_RESULT(18065, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
