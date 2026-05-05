// https://adventofcode.com/2021/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <span>
#include <string>
#include <vector>

using Point3D = aoc::nd_point_type<i32, 3>;
using Scanner = Vec<Point3D>;

fn parse(String const& filename) -> Vec<Scanner> {
  auto scanners = Vec<Scanner>{};
  auto current = Scanner{};
  for (str line : aoc::views::read_lines(filename)) {
    if (line.starts_with("---")) {
      current.clear();
    } else if (line.empty()) {
      scanners.push_back(std::move(current));
    } else {
      current.push_back(aoc::split<Point3D>(line, ','));
    }
  }
  // last scanner has no trailing blank line
  scanners.push_back(std::move(current));
  return scanners;
}

fn solve_case1(std::span<const Scanner> data) -> usize {
  // TODO: Implement Part 1
  return 0;
}

fn solve_case2(std::span<const Scanner> data) -> usize {
  // TODO: Implement Part 2
  return 0;
}

int main() {
  std::println("UNSOLVED");

  std::println("Part 1");
  const auto example = parse("day19.example");
  // AOC_EXPECT_RESULT(79, solve_case1(example));
  const auto input = parse("day19.input");
  // AOC_EXPECT_RESULT(XXX, solve_case1(input));

  std::println("Part 2");
  // AOC_EXPECT_RESULT(XXX, solve_case2(example));
  // AOC_EXPECT_RESULT(XXX, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
