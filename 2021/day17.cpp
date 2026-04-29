// https://adventofcode.com/2021/day/17

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <string>

namespace stdv = std::views;
using Input = std::pair<point, point>;

auto parse(String const& filename) -> Input {
  let s = aoc::read_file(filename);
  let[xs, ys] =
      aoc::split_once(aoc::trim(s).substr(sizeof("target area:")), ", ");
  let x = aoc::split_once<i32>(xs.substr(2), "..");
  let y = aoc::split_once<i32>(ys.substr(2), "..");
  return {
      // Invert the y so that we represent two opposite corners of a rectangle
      {x[0], y[1]},
      {x[1], y[0]},
  };
}

fn solve_case1(Input const& rectangle) -> i32 {
  let max_dy = -rectangle.second.y - 1;
  return (max_dy * (max_dy + 1)) / 2;
}

fn hits_target(i32 dx, i32 dy, point top_left, point bottom_right) -> bool {
  auto p = point{};
  loop {
    p.x += dx;
    p.y += dy;
    dx = std::max(dx - 1, 0);
    dy -= 1;
    if ((p.x >= top_left.x) &&
        (p.x <= bottom_right.x) &&
        (p.y >= bottom_right.y) &&
        (p.y <= top_left.y)) {
      return true;
    }
    if ((p.y < bottom_right.y) || (p.x > bottom_right.x)) {
      return false;
    }
  }
}

fn solve_case2(Input const& rectangle) -> i32 {
  let[top_left, bottom_right] = rectangle;
  let min_dx = [&] {
    auto dx = 0;
    while ((dx * (dx + 1)) / 2 < top_left.x) {
      dx += 1;
    }
    return dx;
  }();
  let max_dx = bottom_right.x;
  let min_dy = bottom_right.y;
  let max_dy = -bottom_right.y - 1; // From part 1

  return std::ranges::count_if(
      stdv::cartesian_product(stdv::iota(min_dx, max_dx + 1),
                              stdv::iota(min_dy, max_dy + 1)),
      [&](let& pair) {
        let[dx, dy] = pair;
        return hits_target(dx, dy, top_left, bottom_right);
      });
}

int main() {
  std::println("Part 1");
  const auto example = parse("day17.example");
  AOC_EXPECT_RESULT(45, solve_case1(example));
  const auto input = parse("day17.input");
  AOC_EXPECT_RESULT(5565, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(112, solve_case2(example));
  AOC_EXPECT_RESULT(2118, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
