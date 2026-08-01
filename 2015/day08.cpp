// https://adventofcode.com/2015/day/8

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

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
  return stdr::count_if(str, [](char c) { return (c == '"') || (c == '\\'); }) +
         2;
}
static_assert(4 == num_encode_chars(R"("")"));
static_assert(4 == num_encode_chars(R"("abc")"));
static_assert(6 == num_encode_chars(R"("aaa\"aaa")"));
static_assert(5 == num_encode_chars(R"("\x27")"));
static_assert(8 == num_encode_chars(R"("\\\\zkisyjpbzandqikqjqvee")"));

using lines_t = std::vector<std::string>;

lines_t parse(const std::string& filename) { return aoc::read_lines(filename); }

int solve_case1(const lines_t& lines) {
  return aoc::ranges::accumulate(lines | stdv::transform(&num_control_chars),
                                 0);
}

int solve_case2(const lines_t& lines) {
  return aoc::ranges::accumulate(lines | stdv::transform(&num_encode_chars), 0);
}

int main() {
  std::println("Part 1");
  const auto example = parse("day08.example");
  AOC_EXPECT_RESULT(12, solve_case1(example));
  const auto input = parse("day08.input");
  AOC_EXPECT_RESULT(1333, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(19, solve_case2(example));
  AOC_EXPECT_RESULT(2046, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
