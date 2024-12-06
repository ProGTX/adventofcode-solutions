// https://adventofcode.com/2024/day/2

#include "../common/common.h"

#include <array>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// A report only counts as safe if both of the following are true:
//   The levels are either all increasing or all decreasing.
//   Any two adjacent levels differ by at least one and at most three.
constexpr bool is_report_safe(std::span<const int> reports) {
  AOC_ASSERT(reports.size() >= 2, "There must be at least 2 reports");
  auto diff = reports[1] - reports[0];
  if (diff == 0) {
    return false;
  }
  const int increasing_factor = aoc::sign(diff);
  for (int i = 1; i < reports.size(); ++i) {
    diff = increasing_factor * (reports[i] - reports[i - 1]);
    if ((diff < 1) || (diff > 3)) {
      return false;
    }
  }
  return true;
}

static_assert(is_report_safe(std::array{7, 6, 4, 2, 1}));
static_assert(!is_report_safe(std::array{1, 2, 7, 8, 9}));
static_assert(!is_report_safe(std::array{9, 7, 6, 2, 1}));
static_assert(!is_report_safe(std::array{1, 3, 2, 4, 5}));
static_assert(!is_report_safe(std::array{8, 6, 4, 4, 1}));
static_assert(is_report_safe(std::array{1, 3, 6, 7, 9}));

template <bool words>
int solve_case(const std::string& filename) {
  int sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto reports = aoc::split<std::vector<int>>(line, ' ');
    sum += static_cast<int>(is_report_safe(reports));
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(2, solve_case<false>("day02.example"));
  AOC_EXPECT_RESULT(502, solve_case<false>("day02.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day02.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
