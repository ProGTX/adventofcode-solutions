// https://adventofcode.com/2015/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <print>
#include <ranges>

template <bool RANGES>
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
      let required = generous_sue.at(key);
      let actual = aoc::to_number<u32>(value);
      if constexpr (!RANGES) {
        return actual == required;
      } else {
        if ((key == "cats") || (key == "trees")) {
          return actual > required;
        } else if ((key == "pomeranians") || (key == "goldfish")) {
          return actual < required;
        } else {
          return actual == required;
        }
      }
    });
    if (matches) {
      return static_cast<u32>(index + 1);
    }
  }
  AOC_UNREACHABLE("Aunt not found!");
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(373, solve_case<false>("day16.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(260, solve_case<true>("day16.input"));
  AOC_RETURN_CHECK_RESULT();
}
