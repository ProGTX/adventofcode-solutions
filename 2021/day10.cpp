// https://adventofcode.com/2021/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const std::string opening_braces = "([{<";
constexpr const std::string closing_braces = ")]}>";
constexpr const std::array illegal_scores = {3, 57, 1197, 25137};

constexpr int syntax_err_score(std::string_view navigation_line) {
  AOC_ASSERT(navigation_line.size() > 0, "Not handling empty strings");
  AOC_ASSERT(closing_braces.find(navigation_line[0]) == std::string::npos,
             "Shouldn't start with a closing brace");
  std::vector<char> closing_braces_stack;
  for (const char c : navigation_line) {
    if (auto pos = closing_braces.find(c); pos != std::string::npos) {
      if (c == closing_braces_stack.back()) {
        aoc::pop_stack(closing_braces_stack);
      } else {
        return illegal_scores[pos];
      }
    } else {
      pos = opening_braces.find(c);
      AOC_ASSERT(pos != std::string::npos, "Invalid character");
      closing_braces_stack.push_back(closing_braces[pos]);
    }
  }
  return 0;
}

template <bool>
int solve_case(const std::string& filename) {
  auto navigation_lines = aoc::views::read_lines(filename) |
                          aoc::ranges::to<std::vector<std::string>>();

  int sum = 0;
  sum = aoc::ranges::accumulate(
      navigation_lines | std::views::transform(&syntax_err_score), 0);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(26397, solve_case<false>("day10.example"));
  AOC_EXPECT_RESULT(319233, solve_case<false>("day10.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(1134, solve_case<true>("day10.example"));
  // AOC_EXPECT_RESULT(856716, solve_case<true>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
