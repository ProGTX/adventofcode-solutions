// https://adventofcode.com/2025/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

using PaperRolls = aoc::char_grid<>;

constexpr let PAPER = '@';
constexpr let EMPTY = '.';

fn accessible_indexes(PaperRolls const& paper_rolls) -> Vec<usize> {
  return //
      paper_rolls |
      std::views::enumerate |
      std::views::filter([&](auto&& tuple) {
        let[linear_index, current] = tuple;
        return //
            (current == PAPER) &&
            (4 > std::ranges::count_if(paper_rolls.all_neighbor_values(
                                           paper_rolls.position(linear_index)),
                                       aoc::equal_to_value{PAPER}));
      }) |
      std::views::keys |
      aoc::ranges::to<Vec<usize>>();
}

auto parse(String const& filename) -> PaperRolls {
  return aoc::read_char_grid(filename);
}

fn solve_case1(PaperRolls const& paper_rolls) -> u32 {
  return accessible_indexes(paper_rolls).size();
}

fn solve_case2(PaperRolls const& input) -> u32 {
  auto paper_rolls = input;
  auto indexes = accessible_indexes(paper_rolls);
  auto total = 0;
  while (!indexes.empty()) {
    total += indexes.size();
    for (let pos : indexes | std::views::transform([&](usize index) {
                     return input.position(index);
                   })) {
      paper_rolls.modify(EMPTY, pos.y, pos.x);
    }
    indexes = accessible_indexes(paper_rolls);
  }
  return total;
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(13, solve_case1(example));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(1553, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(43, solve_case2(example));
  AOC_EXPECT_RESULT(8442, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
