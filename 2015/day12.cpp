// https://adventofcode.com/2015/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <print>
#include <ranges>

fn sum(str s_view) -> i32 {
  auto value = 0;
  while (!s_view.empty()) {
    let digit_it =
        std::ranges::find_if(s_view, [](char c) { return std::isdigit(c); });
    if (digit_it == std::end(s_view)) {
      break;
    }
    let digit_pos = std::distance(std::begin(s_view), digit_it);
    AOC_ASSERT(digit_pos > 0, "Assuming number not at beginning of string");
    let nondigit_it = std::ranges::find_if(
        s_view.substr(digit_pos + 1), [](char c) { return !std::isdigit(c); });
    AOC_ASSERT(nondigit_it != std::end(s_view),
               "Assuming number is never at the end");
    let nondigit_pos = std::distance(digit_it, nondigit_it) + digit_pos;
    let current =
        aoc::to_number<i32>(s_view.substr(digit_pos, nondigit_pos - digit_pos));
    value += (s_view[digit_pos - 1] == '-') ? -current : current;
    s_view = s_view.substr(nondigit_pos);
  }
  return value;
}

fn solve_case(String const& filename) -> i32 {
  auto file = std::ifstream{filename};
  return sum(aoc::read_line(file));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  AOC_EXPECT_RESULT(156366, solve_case("day12.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  // AOC_EXPECT_RESULT(0, solve_case("day12.input"));
  AOC_RETURN_CHECK_RESULT();
}
