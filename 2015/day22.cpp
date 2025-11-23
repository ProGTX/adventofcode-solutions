// https://adventofcode.com/2015/day/22

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

struct Boss {
  u16 hp;
  u16 damage;
};

fn parse(String const& filename) -> Boss {
  auto lines_view = aoc::views::read_lines(filename);
  auto line_it = std::ranges::begin(lines_view);
  let parse_line = [&] {
    let[_, value_str] = aoc::split_once(*line_it, ": ");
    let value = aoc::to_number<u16>(value_str);
    ++line_it;
    return value;
  };
  return Boss{
      .hp = parse_line(),
      .damage = parse_line(),
  };
}

template <bool FULL_INPUT, bool HARD_MODE>
fn solve_case(Boss const& boss) -> u32 {
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(226, (solve_case<false, false>(example)));
  let example2 = parse("day22.example2");
  AOC_EXPECT_RESULT(641, (solve_case<false, false>(example2)));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(953, (solve_case<true, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(226, (solve_case<false, true>(example)));
  AOC_EXPECT_RESULT(588, (solve_case<false, true>(example2)));
  AOC_EXPECT_RESULT(1289, (solve_case<true, true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
