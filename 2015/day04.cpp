// https://adventofcode.com/2015/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <format>
#include <print>

auto parse(String const& filename) -> String {
  return String{aoc::trim(aoc::read_file(filename))};
}

template <bool Part2>
fn solve_case(str key) -> u32 {
  for (u32 n = 0;; ++n) {
    let full_key = std::format("{}{}", key, n);
    let hash = aoc::md5(full_key);
    if (hash[0] == 0 && hash[1] == 0 && (hash[2] & 0xF0u) == 0) {
      if constexpr (Part2) {
        if ((hash[2] & 0x0Fu) > 0) {
          continue;
        }
      }
      return n;
    }
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(609043, solve_case<false>(example));
  AOC_EXPECT_RESULT(1048970, solve_case<false>("pqrstuv"));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(254575, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6742839, solve_case<true>(example));
  AOC_EXPECT_RESULT(5714438, solve_case<true>("pqrstuv"));
  AOC_EXPECT_RESULT(1038736, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
