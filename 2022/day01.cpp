// https://adventofcode.com/2022/day/1

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

fn parse(String const& filename) -> Vec<u32> {
  return aoc::split(aoc::trim(aoc::read_file(filename)), "\n\n") |
         std::views::transform([](str food) {
           return aoc::ranges::accumulate(aoc::split_to_vec<u32>(food, '\n'),
                                          0u);
         }) |
         aoc::ranges::to<Vec<u32>>();
}

fn solve_case1(std::span<const u32> elves) -> u32 {
  return std::ranges::max(elves);
}

fn solve_case2(Vec<u32> elves) -> u32 {
  std::ranges::sort(elves, std::greater{});
  return aoc::ranges::accumulate(elves | std::views::take(3), 0u);
}

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(24000, solve_case1(example));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(75622, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(45000, solve_case2(example));
  AOC_EXPECT_RESULT(213159, solve_case2(input));
  AOC_RETURN_CHECK_RESULT();
}
