// https://adventofcode.com/2015/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <span>

using String10T = aoc::static_vector<String, 10>;
using ReplacementsT = std::map<String, String10T>;

fn parse(String const& filename) -> std::pair<ReplacementsT, String> {
  auto replacements = ReplacementsT{};
  auto last = false;
  for (str line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (last) {
      return {replacements, String{line}};
    }
    if (line.empty()) {
      last = true;
      continue;
    }
    let[needle, replace] = aoc::split_once<String>(line, " => ");
    auto [it, _] = replacements.emplace(needle, String10T{});
    it->second.push_back(replace);
  }
  AOC_UNREACHABLE("Last line is supposed to terminate the loop");
}

fn solve_case1(auto&& parse_input) -> usize {
  let & [ replacements, molecule ] = parse_input;
  auto generated = std::set<String>{};
  for (let& [ needle, replaces ] : replacements) {
    for (let[index, _] : molecule | aoc::views::match_indices(needle)) {
      for (let& replace : replaces) {
        generated.insert(molecule.substr(0, index) +
                         replace +
                         molecule.substr(index + needle.size()));
      }
    }
  }
  return generated.size();
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(4, solve_case1(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(576, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3, solve_case2(example));
  // AOC_EXPECT_RESULT(1257, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
