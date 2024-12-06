// https://adventofcode.com/2024/day/1

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

constexpr int sum_diffs(std::vector<int> left_list,
                        std::vector<int> right_list) {
  AOC_ASSERT(left_list.size() == right_list.size(), "Lists must be same size");
  std::ranges::sort(left_list);
  std::ranges::sort(right_list);

  int sum = 0;
  for (int i = 0; i < left_list.size(); i++) {
    sum += aoc::abs(right_list[i] - left_list[i]);
  }
  return sum;
}

static_assert(sum_diffs({2}, {7}) == 5);
static_assert(sum_diffs({3, 4, 2, 1, 3, 3}, {4, 3, 5, 3, 9, 3}) == 11);

template <bool words>
int solve_case(const std::string& filename) {
  std::vector<int> left_list;
  std::vector<int> right_list;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [left, right] =
        aoc::split<std::array<int, 2>>(line, std::string_view("   "));
    left_list.push_back(left);
    right_list.push_back(right);
  }

  auto sum = sum_diffs(left_list, right_list);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(11, solve_case<false>("day01.example"));
  AOC_EXPECT_RESULT(3569916, solve_case<false>("day01.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day01.example2"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
