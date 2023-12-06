// https://adventofcode.com/2023/day/1

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

static constexpr auto numbers_view =
    std::views::iota('0', static_cast<char>('9' + 1));

constexpr int find_number(std::string_view str) {
  auto first_it = std::ranges::find_first_of(str, numbers_view);
  auto last_it =
      std::ranges::find_first_of(str | std::views::reverse, numbers_view);
  int number =
      static_cast<int>(*first_it - '0') * 10 + static_cast<int>(*last_it - '0');
  return number;
}

int solve_case(const std::string& filename) {
  int sum = 0;

  auto solver = [&](std::string_view line) { sum += find_number(line); };

  readfile_op(filename, solver);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(142, solve_case("day01.example"));
  AOC_EXPECT_RESULT(54388, solve_case("day01.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(142, solve_case("day01.example"));
  // AOC_EXPECT_RESULT(54388, solve_case("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
