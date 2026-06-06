// https://adventofcode.com/2022/day/6

#include "../common/common.h"
#include "../common/rust.h"

#include <bit>
#include <cstdint>
#include <print>

// Returns the 1-indexed position after the first WINDOW_SIZE chars
// where all chars are distinct
template <u32 WINDOW_SIZE>
fn find_marker(str s) -> i32 {
  for (auto i = 0uz; i + WINDOW_SIZE <= s.size(); ++i) {
    // Set one bit per letter (a=bit 0, b=bit 1, ...).
    auto mask = u32{};
    for (char c : s.substr(i, WINDOW_SIZE)) {
      mask |= 1u << (c - 'a');
    }
    // If all WINDOW_SIZE chars are distinct,
    // exactly WINDOW_SIZE bits will be set
    if (std::popcount(mask) == WINDOW_SIZE) {
      // Convert window start index to end position (1-indexed)
      return i + WINDOW_SIZE;
    }
  }
  return -1;
}

template <u32 WINDOW_SIZE>
fn solve_case(String const& filename) -> Vec<i32> {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return find_marker<WINDOW_SIZE>(line); }) |
         aoc::ranges::to<Vec<i32>>();
}

int main() {
  std::println("Part 1");
  let example4 = solve_case<4>("day06.example");
  AOC_EXPECT_RESULT(7, example4[0]);
  AOC_EXPECT_RESULT(5, example4[1]);
  AOC_EXPECT_RESULT(6, example4[2]);
  AOC_EXPECT_RESULT(10, example4[3]);
  AOC_EXPECT_RESULT(11, example4[4]);
  let input4 = solve_case<4>("day06.input");
  AOC_EXPECT_RESULT(1361, input4[0]);

  std::println("Part 2");
  let example14 = solve_case<14>("day06.example");
  AOC_EXPECT_RESULT(19, example14[0]);
  AOC_EXPECT_RESULT(23, example14[1]);
  AOC_EXPECT_RESULT(23, example14[2]);
  AOC_EXPECT_RESULT(29, example14[3]);
  AOC_EXPECT_RESULT(26, example14[4]);
  let input14 = solve_case<14>("day06.input");
  AOC_EXPECT_RESULT(3263, input14[0]);

  AOC_RETURN_CHECK_RESULT();
}
