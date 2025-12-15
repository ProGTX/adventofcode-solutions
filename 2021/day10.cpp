// https://adventofcode.com/2021/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;
constexpr const std::string_view opening_braces = "([{<";
constexpr const std::string_view closing_braces = ")]}>";
constexpr const std::array illegal_scores = {3, 57, 1197, 25137};

template <bool autocomplete>
constexpr int_t get_score(std::string_view navigation_line) {
  AOC_ASSERT(navigation_line.size() > 0, "Not handling empty strings");
  AOC_ASSERT(closing_braces.find(navigation_line[0]) == std::string::npos,
             "Shouldn't start with a closing brace");
  std::vector<char> closing_braces_stack;
  for (const char c : navigation_line) {
    if (auto pos = closing_braces.find(c); pos != std::string::npos) {
      if (c == closing_braces_stack.back()) {
        closing_braces_stack.pop_back();
      } else if constexpr (!autocomplete) {
        return illegal_scores[pos];
      } else {
        return 0;
      }
    } else {
      pos = opening_braces.find(c);
      AOC_ASSERT(pos != std::string::npos, "Invalid character");
      closing_braces_stack.push_back(closing_braces[pos]);
    }
  }
  int_t score = 0;
  if constexpr (autocomplete) {
    for (const char brace : closing_braces_stack | std::views::reverse) {
      score *= 5;
      score += closing_braces.find(brace) + 1;
    }
  }
  return score;
}

static_assert(294 == get_score<true>("<{([{{}}[<[[[<>{}]]]>[]]"));

template <bool autocomplete>
int_t solve_case(const std::string& filename) {
  auto navigation_lines = aoc::views::read_lines(filename) |
                          aoc::ranges::to<std::vector<std::string>>();

  int_t sum = 0;
  auto scores = navigation_lines |
                std::views::transform(&get_score<autocomplete>) |
                aoc::ranges::to<std::vector<int_t>>();
  if constexpr (!autocomplete) {
    sum = aoc::ranges::accumulate(scores, 0);
  } else {
    std::ranges::sort(scores);
    const auto non_zero_it =
        std::ranges::find_if(scores, aoc::not_equal_to_value{0});
    const auto offset = std::distance(std::begin(scores), non_zero_it);
    sum = scores[offset + (scores.size() - offset) / 2];
  }
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(26397, solve_case<false>("day10.example"));
  AOC_EXPECT_RESULT(319233, solve_case<false>("day10.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(288957, solve_case<true>("day10.example"));
  AOC_EXPECT_RESULT(1118976874, solve_case<true>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
