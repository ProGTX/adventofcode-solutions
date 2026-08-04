// https://adventofcode.com/2015/day/2

#include "../common/common.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#endif

using dims_t = aoc::nd_point_type<int, 3>;
using boxes_t = std::vector<dims_t>;

boxes_t parse(const std::string& filename) {
  auto boxes = boxes_t{};
  for (std::string_view line : aoc::views::read_lines(filename)) {
    boxes.push_back(aoc::split<dims_t>(line, 'x'));
  }
  return boxes;
}

int solve_case1(const boxes_t& boxes) {
  int sum = 0;
  for (const auto& box : boxes) {
    auto sides = dims_t{box[0] * box[1], box[0] * box[2], box[1] * box[2]};
    sum += sides.dot(dims_t{2, 2, 2}) + stdr::min(sides);
  }
  return sum;
}

int solve_case2(const boxes_t& boxes) {
  int ribbon = 0;
  for (auto box : boxes) {
    stdr::sort(box);
    ribbon += 2 * (box[0] + box[1]) + box[0] * box[1] * box[2];
  }
  return ribbon;
}

int main() {
  std::println("Part 1");
  const auto example = parse("day02.example");
  AOC_EXPECT_RESULT((58 + 43), solve_case1(example));
  const auto input = parse("day02.input");
  AOC_EXPECT_RESULT(1606483, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT((34 + 14), solve_case2(example));
  AOC_EXPECT_RESULT(3842356, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
