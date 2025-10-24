// https://adventofcode.com/2015/day/5

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

constexpr bool contains_three_vowels(std::string_view str) {
  int count = 0;
  for (char c : str) {
    count += static_cast<int>(std::ranges::contains("aeiou", c));
    if (count >= 3) {
      return true;
    }
  }
  return false;
}

using namespace std::string_view_literals;
static constexpr std::array bad_strings = {"ab"sv, "cd"sv, "pq"sv, "xy"sv};

constexpr bool double_letters_ok(std::string_view str) {
  bool repeated = false;
  for (auto&& pair_raw : str | std::views::slide(2)) {
    const auto pair = std::string_view{pair_raw};
    if (std::ranges::contains(bad_strings, pair)) {
      return false;
    }
    repeated |= (pair[0] == pair[1]);
  }
  return repeated;
}

constexpr bool is_nice1(std::string_view str) {
  return contains_three_vowels(str) && double_letters_ok(str);
}

constexpr bool is_nice2(const std::string_view str) {
  bool pair_repeats = false;
  bool letter_repeats = false;
  // Skip last char
  for (const auto [index, c] :
       str.substr(0, str.size() - 1) | std::views::enumerate) {
    const auto pair = str.substr(index, 2);
    const auto rest = str.substr(index + 2);
    pair_repeats |= rest.contains(pair);
    letter_repeats |= (rest.size() > 0) && (c == rest[0]);
  }
  return pair_repeats && letter_repeats;
}

static_assert(is_nice2("qjhvhtzxzqqjkmpb"));
static_assert(is_nice2("xxyxx"));
static_assert(!is_nice2("uurcxstgmygtbstg"));
static_assert(!is_nice2("ieodomkazucvgmuy"));

int solve_case1(const std::string& filename) {
  return std::ranges::count_if(aoc::views::read_lines(filename), &is_nice1);
}

int solve_case2(const std::string& filename) {
  return std::ranges::count_if(aoc::views::read_lines(filename), &is_nice2);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(2, solve_case1("day05.example"));
  AOC_EXPECT_RESULT(258, solve_case1("day05.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(1, solve_case2("day05.example"));
  AOC_EXPECT_RESULT(53, solve_case2("day05.input"));
  AOC_RETURN_CHECK_RESULT();
}
