// https://adventofcode.com/2021/day/22

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cstdio>
#include <print>
#include <span>
#include <string>
#include <vector>

namespace stdv = std::views;

using RangeInclusive = aoc::closed_range<i32>;

struct Cuboid {
  RangeInclusive x;
  RangeInclusive y;
  RangeInclusive z;
  i32 sign;

  fn size() const -> u64 {
    return static_cast<u64>(x.end - x.begin + 1) *
           static_cast<u64>(y.end - y.begin + 1) *
           static_cast<u64>(z.end - z.begin + 1);
  }
};

using Input = Vec<Cuboid>;

fn parse(String const& filename) -> Input {
  auto cuboids = Input{};

  let parse_range = [](str s) {
    let[_, nums] = aoc::split_once(s, '=');
    let range = aoc::split<point>(nums, "..");
    return RangeInclusive{range.x, range.y};
  };

  for (str line : aoc::views::read_lines(filename)) {
    let[state, coords] = aoc::split_once(line, ' ');
    let sign = state == "on" ? 1 : -1;

    auto parts = aoc::split_to_array<3>(coords, ',');
    cuboids.emplace_back(Cuboid{
        .x = parse_range(parts[0]),
        .y = parse_range(parts[1]),
        .z = parse_range(parts[2]),
        .sign = sign,
    });
  }

  return cuboids;
}

fn intersect(Cuboid const& a, Cuboid const& b) -> Option<Cuboid> {
  let x0 = std::max(a.x.begin, b.x.begin);
  let x1 = std::min(a.x.end, b.x.end);
  let y0 = std::max(a.y.begin, b.y.begin);
  let y1 = std::min(a.y.end, b.y.end);
  let z0 = std::max(a.z.begin, b.z.begin);
  let z1 = std::min(a.z.end, b.z.end);
  // aoc::closed_range sorts the begin and end,
  // so the checks needs to be performed before constructing ranges
  if ((x0 <= x1) && (y0 <= y1) && (z0 <= z1)) {
    return Cuboid{RangeInclusive{x0, x1}, RangeInclusive{y0, y1},
                  RangeInclusive{z0, z1}, -a.sign};
  }
  return None;
}

fn clamp_cuboid(Cuboid const& c, i32 lo, i32 hi) -> Option<Cuboid> {
  let x0 = std::max(c.x.begin, lo);
  let x1 = std::min(c.x.end, hi);
  let y0 = std::max(c.y.begin, lo);
  let y1 = std::min(c.y.end, hi);
  let z0 = std::max(c.z.begin, lo);
  let z1 = std::min(c.z.end, hi);
  // aoc::closed_range sorts the begin and end,
  // so the checks needs to be performed before constructing ranges
  if ((x0 <= x1) && (y0 <= y1) && (z0 <= z1)) {
    return Cuboid{RangeInclusive{x0, x1}, RangeInclusive{y0, y1},
                  RangeInclusive{z0, z1}, c.sign};
  }
  return None;
}

fn reboot(std::span<Cuboid const> steps) -> u64 {
  auto cuboids = Vec<Cuboid>{};
  for (let& step : steps) {
    auto new_cuboids = cuboids;
    for (let& c : cuboids) {
      if (let i = intersect(c, step)) {
        new_cuboids.push_back(*i);
      }
    }
    if (step.sign > 0) {
      new_cuboids.push_back(step);
    }
    cuboids = std::move(new_cuboids);
  }
  return static_cast<u64>(aoc::ranges::accumulate( //
      cuboids | stdv::transform(
                    [](let& c) { return static_cast<i64>(c.size()) * c.sign; }),
      i64{}));
}

fn solve_case1(std::span<Cuboid const> steps) -> u64 {
  auto clamped = Vec<Cuboid>{};
  for (let& s : steps) {
    if (let c = clamp_cuboid(s, -50, 50)) {
      clamped.push_back(*c);
    }
  }
  return reboot(clamped);
}

fn solve_case2(std::span<Cuboid const> steps) -> u64 { return reboot(steps); }

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(39ULL, solve_case1(example));
  let example2 = parse("day22.example2");
  AOC_EXPECT_RESULT(590784ULL, solve_case1(example2));
  let example3 = parse("day22.example3");
  AOC_EXPECT_RESULT(474140ULL, solve_case1(example3));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(596989ULL, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(39ULL, solve_case2(example));
  AOC_EXPECT_RESULT(39769202357779ULL, solve_case2(example2));
  AOC_EXPECT_RESULT(2758514936282235ULL, solve_case2(example3));
  AOC_EXPECT_RESULT(1160011199157381ULL, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
