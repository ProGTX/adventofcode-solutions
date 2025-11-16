// https://adventofcode.com/2015/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <print>
#include <ranges>

fn solve_case(String const& filename) -> u32 {
  let generous_sue = aoc::flat_map<str, u32>{
      {"children", 3}, {"cats", 7},     {"samoyeds", 2}, {"pomeranians", 3},
      {"akitas", 0},   {"vizslas", 0},  {"goldfish", 5}, {"trees", 3},
      {"cars", 2},     {"perfumes", 1},
  };

  for (let&& [ index, line ] :
       aoc::views::read_lines(filename) | std::views::enumerate) {
    let all_info = aoc::split_once(line, ": ");
    let split_info = aoc::split_to_array<3>(all_info[1], ", ");
    let matches = std::ranges::all_of(split_info, [&](str info) {
      let[key, value] = aoc::split_once(info, ": ");
      return generous_sue.at(key) == aoc::to_number<u32>(value);
    });
    if (matches) {
      return static_cast<u32>(index + 1);
    }
  }
  AOC_ASSERT(false, "Aunt not found!");
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(373, solve_case("day16.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(1766400, solve_case<true>("day16.input"));
  AOC_RETURN_CHECK_RESULT();
}
