// https://adventofcode.com/2015/day/1

#include "../common/common.h"

#ifndef AOC_IMPORT_STD
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#endif

std::string parse(const std::string& filename) {
  return aoc::read_single_line(filename);
}

template <bool check_position>
int solve_case(std::string_view directions) {
  int floor = 0;
  [[maybe_unused]] int position = 0;
  for (char c : directions) {
    floor += static_cast<int>(c == '(') - static_cast<int>(c == ')');
    if constexpr (check_position) {
      ++position;
      if (floor == -1) {
        break;
      }
    }
  }
  return check_position ? position : floor;
}

int main() {
  std::println("Part 1");
  const auto example = parse("day01.example");
  AOC_EXPECT_RESULT(-1, solve_case<false>(example));
  const auto input = parse("day01.input");
  AOC_EXPECT_RESULT(280, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(5, solve_case<true>(example));
  AOC_EXPECT_RESULT(1797, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
