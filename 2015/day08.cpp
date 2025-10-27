// https://adventofcode.com/2015/day/8

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

constexpr int num_control_chars(std::string_view str) {
  AOC_ASSERT((str.size() >= 2) && (str.front() == '"') && (str.back() == '"'),
             "Doesn't contain a string literal");
  int count = 0;
  while (!str.empty()) {
    const auto backslash = str.find('\\');
    if (backslash == std::string::npos) {
      break;
    }
    const auto skip = [&]() {
      switch (str[backslash + 1]) {
        case 'x':
          count += 3;
          return 4;
        default:
          count += 1;
          return 2;
      }
    }();
    str = str.substr(backslash + skip);
  }
  return count + 2;
}
static_assert(2 == num_control_chars(R"("")"));
static_assert(2 == num_control_chars(R"("abc")"));
static_assert(3 == num_control_chars(R"("aaa\"aaa")"));
static_assert(5 == num_control_chars(R"("\x27")"));
static_assert(4 == num_control_chars(R"("\\\\zkisyjpbzandqikqjqvee")"));

constexpr int num_encode_chars(std::string_view str) {
  return std::ranges::count_if(
             str, [](char c) { return (c == '"') || (c == '\\'); }) +
         2;
}
static_assert(4 == num_encode_chars(R"("")"));
static_assert(4 == num_encode_chars(R"("abc")"));
static_assert(6 == num_encode_chars(R"("aaa\"aaa")"));
static_assert(5 == num_encode_chars(R"("\x27")"));
static_assert(8 == num_encode_chars(R"("\\\\zkisyjpbzandqikqjqvee")"));

int solve_case1(const std::string& filename) {
  return aoc::ranges::accumulate(aoc::views::read_lines(filename) |
                                     std::views::transform(&num_control_chars),
                                 0);
}

int solve_case2(const std::string& filename) {
  return aoc::ranges::accumulate(aoc::views::read_lines(filename) |
                                     std::views::transform(&num_encode_chars),
                                 0);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(12, solve_case1("day08.example"));
  AOC_EXPECT_RESULT(1333, solve_case1("day08.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(19, solve_case2("day08.example"));
  AOC_EXPECT_RESULT(2046, solve_case2("day08.input"));
  AOC_RETURN_CHECK_RESULT();
}
