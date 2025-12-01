// https://adventofcode.com/2025/day/1

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

fn parse(String const& filename) -> Vec<i32> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           auto distance = aoc::to_number<i32>(line.substr(1));
           if (line[0] == 'L') {
             distance = -distance;
           }
           return distance;
         }) |
         aoc::ranges::to<Vec<i32>>();
}

fn solve_case1(std::span<const i32> sequence) -> i32 {
  auto current = 50;
  return aoc::ranges::accumulate(
      sequence | std::views::transform([&](i32 rotation) {
        // Not fully correct calculation for the modulo,
        // but it identifies zeros correctly
        current = (current + rotation) % 100;
        return static_cast<i32>(current == 0);
      }),
      0);
}

fn solve_case2(std::span<const i32> sequence) -> i32 {
  auto current = 50;
  return aoc::ranges::accumulate(
      sequence | std::views::transform([&](i32 rotation) {
        let current_zero = (current == 0);
        current += rotation;
        let num_zeros = aoc::abs(current / 100) +
                        static_cast<i32>((current <= 0) && !current_zero);
        // Here we have to use the full formula for the modulo
        current = ((current % 100) + 100) % 100;
        return num_zeros;
      }),
      0);
}

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(3, solve_case1(example));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(1048, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_case2(example));
  AOC_EXPECT_RESULT(6498, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
