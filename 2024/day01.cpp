// https://adventofcode.com/2024/day/1

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

constexpr int sum_diffs(std::vector<int> left_list,
                        std::vector<int> right_list) {
  AOC_ASSERT(left_list.size() == right_list.size(), "Lists must be same size");
  std::ranges::sort(left_list);
  std::ranges::sort(right_list);

  int sum = 0;
  for (int i = 0; i < left_list.size(); ++i) {
    sum += aoc::abs(right_list[i] - left_list[i]);
  }
  return sum;
}

static_assert(sum_diffs({2}, {7}) == 5);
static_assert(sum_diffs({3, 4, 2, 1, 3, 3}, {4, 3, 5, 3, 9, 3}) == 11);

constexpr int similarity_score(std::vector<int> left_list,
                               std::vector<int> right_list) {
  std::ranges::sort(left_list);
  std::ranges::sort(right_list);

  int previous = 0;
  int multiplier = 0;
  auto right_start_it = std::begin(right_list);
  const auto right_end_it = std::end(right_list);

  int sum = 0;
  for (int i = 0; i < left_list.size(); ++i) {
    const auto current = left_list[i];
    if (current == previous) {
      sum += current * multiplier;
      continue;
    }

    auto match_it = std::ranges::find(right_start_it, right_end_it, current);
    multiplier = 0;
    while (match_it != right_end_it) {
      if (*match_it != current) {
        break;
      }
      ++multiplier;
      ++match_it;
      right_start_it = match_it;
    }

    sum += current * multiplier;
    previous = current;
  }

  return sum;
}

static_assert(similarity_score({3}, {4, 3, 5, 3, 9, 3}) == 9);
static_assert(similarity_score({3, 3}, {4, 3, 5, 3, 9, 3}) == 18);
static_assert(similarity_score({3, 4}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 3}, {4, 3, 5, 3, 9, 3}) == 22);
static_assert(similarity_score({3, 4, 2}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 2, 1}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 2, 1, 3}, {4, 3, 5, 3, 9, 3}) == 22);
static_assert(similarity_score({3, 4, 2, 1, 3, 3}, {4, 3, 5, 3, 9, 3}) == 31);

template <bool similarity>
int solve_case(const std::string& filename) {
  std::vector<int> left_list;
  std::vector<int> right_list;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [left, right] =
        aoc::split<std::array<int, 2>>(line, std::string_view("   "));
    left_list.push_back(left);
    right_list.push_back(right);
  }

  int sum = 0;
  if constexpr (similarity) {
    sum = similarity_score(left_list, right_list);
  } else {
    sum = sum_diffs(left_list, right_list);
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(11, solve_case<false>("day01.example"));
  AOC_EXPECT_RESULT(3569916, solve_case<false>("day01.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(31, solve_case<true>("day01.example"));
  AOC_EXPECT_RESULT(26407426, solve_case<true>("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
