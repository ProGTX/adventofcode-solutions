// https://adventofcode.com/2021/day/1

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

fn parse(String const& filename) -> Vec<u32> {
  return aoc::views::read_numbers<u32>(filename) | aoc::ranges::to<Vec<u32>>();
}

fn solve_case1(std::span<const u32> depths) -> usize {
  return std::ranges::count_if(
      depths | std::views::slide(2),
      [](auto&& window) { return window[1] > window[0]; });
}

fn solve_case2(std::span<const u32> depths) -> usize {
  return solve_case1(depths |
                     std::views::slide(3) |
                     std::views::transform([](auto&& window) {
                       return aoc::ranges::accumulate(window, 0u);
                     }) |
                     aoc::ranges::to<Vec<u32>>());
}

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(7, solve_case1(example));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(1228, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(5, solve_case2(example));
  AOC_EXPECT_RESULT(1257, solve_case2(input));
  AOC_RETURN_CHECK_RESULT();
}
