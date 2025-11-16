// https://adventofcode.com/2022/day/4

#include "../common/common.h"

#include <array>
#include <iostream>
#include <string>
#include <string_view>

struct range : public aoc::closed_range<int> {
  using base_t = aoc::closed_range<int>;
  constexpr range(std::array<int, 2> range_array)
      : base_t{range_array[0], range_array[1]} {}
};

template <bool use_overlaps>
int solve_case(const std::string& filename) {
  int score = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [firstElfStr, secondElfStr] = aoc::split_once(line, ',');

    auto firstElf = range{aoc::split_once<int>(firstElfStr, '-')};
    auto secondElf = range{aoc::split_once<int>(secondElfStr, '-')};

    if constexpr (use_overlaps) {
      if (firstElf.overlaps_with(secondElf)) {
        score += 1;
      }
    } else {
      if (firstElf.contains(secondElf) || secondElf.contains(firstElf)) {
        score += 1;
      }
    }
  }

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(2, solve_case<false>("day04.example"));
  AOC_EXPECT_RESULT(582, solve_case<false>("day04.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(4, solve_case<true>("day04.example"));
  AOC_EXPECT_RESULT(893, solve_case<true>("day04.input"));
  AOC_RETURN_CHECK_RESULT();
}
