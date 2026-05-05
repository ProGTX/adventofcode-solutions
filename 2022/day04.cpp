// https://adventofcode.com/2022/day/4

#include "../common/common.h"

#include <array>
#include <print>
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

  return score;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(2, solve_case<false>("day04.example"));
  AOC_EXPECT_RESULT(582, solve_case<false>("day04.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(4, solve_case<true>("day04.example"));
  AOC_EXPECT_RESULT(893, solve_case<true>("day04.input"));
  AOC_RETURN_CHECK_RESULT();
}
