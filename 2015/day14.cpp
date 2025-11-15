// https://adventofcode.com/2015/day/14

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

struct Reindeer {
  u32 kms;
  u32 fly;
  u32 rest;
};

fn parse(String const& filename) -> Vec<Reindeer> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           let words = aoc::split<std::array<str, 14>>(line, ' ');
           return Reindeer{
               .kms = aoc::to_number<u32>(words[3]),
               .fly = aoc::to_number<u32>(words[6]),
               .rest = aoc::to_number<u32>(words[13]),
           };
         }) |
         aoc::ranges::to<Vec<Reindeer>>();
}

template <u32 SECONDS>
fn solve_case(String const& filename) -> u32 {
  return std::ranges::max(
      parse(filename) | std::views::transform([](Reindeer const& reindeer) {
        let cycle_time = reindeer.fly + reindeer.rest;
        let num_cycles = SECONDS / cycle_time;
        let remainder = SECONDS % cycle_time;
        let distance = num_cycles * reindeer.fly * reindeer.kms +
                       std::min(remainder, reindeer.fly) * reindeer.kms;
        return distance;
      }));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(1120, solve_case<1000>("day14.example"));
  AOC_EXPECT_RESULT(2660, solve_case<2503>("day14.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(4, solve_case<true>("day14.example"));
  // AOC_EXPECT_RESULT(96852, solve_case<true>("day14.input"));
  AOC_RETURN_CHECK_RESULT();
}
