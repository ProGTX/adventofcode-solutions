// https://adventofcode.com/2022/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>
#include <span>
#include <string_view>

struct range : public aoc::closed_range<int> {
  using base_t = aoc::closed_range<int>;
  constexpr range(std::array<int, 2> range_array)
      : base_t{range_array[0], range_array[1]} {}
};

using Input = Vec<std::pair<range, range>>;

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](std::string_view line) {
           auto [firstElfStr, secondElfStr] = aoc::split_once(line, ',');
           return std::pair{range{aoc::split_once<int>(firstElfStr, '-')},
                            range{aoc::split_once<int>(secondElfStr, '-')}};
         }) |
         aoc::ranges::to<Input>();
}

template <bool use_overlaps>
fn solve_case(std::span<const std::pair<range, range>> input) -> u32 {
  u32 score = 0;
  for (let& [ firstElf, secondElf ] : input) {
    if constexpr (use_overlaps) {
      score += static_cast<u32>(firstElf.overlaps_with(secondElf));
    } else {
      score += static_cast<u32>(firstElf.contains(secondElf) ||
                                secondElf.contains(firstElf));
    }
  }
  return score;
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(2, solve_case<false>(example));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(582, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4, solve_case<true>(example));
  AOC_EXPECT_RESULT(893, solve_case<true>(input));
  AOC_RETURN_CHECK_RESULT();
}
