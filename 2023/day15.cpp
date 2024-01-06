// https://adventofcode.com/2023/day/15

#include "../common/common.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

// Determine the ASCII code for the current character of the string.
// Increase the current value by the ASCII code you just determined.
// Set the current value to itself multiplied by 17.
// Set the current value to the remainder of dividing itself by 256.
constexpr int hash_alg(std::string_view str) {
  return ranges::fold_left(str | views::transform_cast<int>(), 0,
                           [](int current_value, int ascii) {
                             current_value += ascii;
                             current_value *= 17;
                             current_value %= 256;
                             return current_value;
                           });
}
static_assert(hash_alg("HASH") == 52);

using steps_t = std::vector<std::string>;

constexpr int sum_steps(const steps_t& steps) {
  return ranges::accumulate(steps | std::views::transform(hash_alg), 0);
}

constexpr steps_t test_case() {
  return {
      "rn=1", "cm-",  "qp=3", "cm=2", "qp-",  "pc=4",
      "ot=9", "ab=5", "pc-",  "pc=6", "ot=7",
  };
}
static_assert(1320 == sum_steps(test_case()));

template <bool>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  steps_t steps;

  auto read_values = [&](std::string_view line) {
    steps = split<steps_t>(line, ',');
  };
  readfile_op(filename, read_values);

  int sum = sum_steps(steps);
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1320, (solve_case<false>("day15.example")));
  AOC_EXPECT_RESULT(510273, (solve_case<false>("day15.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(64, (solve_case<true>("day15.example")));
  // AOC_EXPECT_RESULT(95273, (solve_case<true>("day15.input")));
  AOC_RETURN_CHECK_RESULT();
}
