// https://adventofcode.com/2024/day/3

#include "../common/common.h"

#include <iostream>
#include <string>
#include <string_view>

static constexpr int max_digits = 3;

constexpr int parse_multiplications(std::string_view line) {
  constexpr std::string_view prefix = "mul(";
  // Plus 1 to simplify bounds checking
  aoc::static_vector<char, max_digits + 1> number_buffer;
  int left = 0;
  int right = 0;
  int current_prefix_i = 0;
  int sum = 0;
  for (char elem : line) {
    if (current_prefix_i < prefix.size()) {
      // Getting through the prefix
      if (elem == prefix[current_prefix_i]) {
        ++current_prefix_i;
      } else {
        current_prefix_i = 0;
      }
    } else if (left == 0) {
      // Parsing first number
      if (aoc::is_number(elem)) {
        number_buffer.push_back(elem);
        if (number_buffer.size() <= max_digits) {
          continue;
        }
      } else if (elem == ',') {
        if (number_buffer.size() > 0) {
          // Parse the number
          left = aoc::to_int_naive<int>(
              std::string_view(number_buffer.data(), number_buffer.size()));
          number_buffer.clear();
          continue;
        }
      }
      // If nothing succeeded, return to beginning
      current_prefix_i = 0;
      number_buffer.clear();
    } else if (right == 0) {
      // Parsing second number
      if (aoc::is_number(elem)) {
        number_buffer.push_back(elem);
        if (number_buffer.size() <= max_digits) {
          continue;
        }
      } else if (elem == ')') {
        if (number_buffer.size() > 0) {
          // Parse the number
          right = aoc::to_int_naive<int>(
              std::string_view(number_buffer.data(), number_buffer.size()));

          // Great success
          sum += left * right;

          // Don't continue, return to beginning
        }
      }
      // If nothing succeeded, return to beginning
      current_prefix_i = 0;
      number_buffer.clear();
      left = 0;
      right = 0;
    } else {
      AOC_ASSERT(false, "Invalid condition");
    }
  }
  return sum;
}

static_assert(parse_multiplications("mul(44,46)") == 2024);

template <bool>
int solve_case(const std::string& filename) {
  int sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    sum += parse_multiplications(line);
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(161, solve_case<false>("day03.example"));
  AOC_EXPECT_RESULT(174960292, solve_case<false>("day03.input"));
  //   std::cout << "Part 2" << std::endl;
  //   AOC_EXPECT_RESULT(281, solve_case<true>("day03.example"));
  //   AOC_EXPECT_RESULT(53515, solve_case<true>("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
