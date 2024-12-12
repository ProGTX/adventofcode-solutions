// https://adventofcode.com/2024/day/7

#include "../common/common.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

// The individual operands can be 32-bit ints
using operands_t = aoc::static_vector<int, 12>;

constexpr bool use_multiply(int evaluation_id, int bit_pos) {
  return (evaluation_id >> bit_pos) & 1;
}

constexpr int_t evaluate_equation(int_t test_value,
                                  std::span<const int> operands) {
  const auto max_operators = operands.size() - 1;
  const int max_evaluations = 1 << max_operators;
  for (int id = 0; id < max_evaluations; ++id) {
    int_t sum = operands[0];
    for (int bit_pos = max_operators - 1; bit_pos >= 0; --bit_pos) {
      if (use_multiply(id, bit_pos)) {
        sum *= operands[max_operators - bit_pos];
      } else {
        sum += operands[max_operators - bit_pos];
      }
    }
    if (sum == test_value) {
      return sum;
    }
  }
  return 0;
}

static_assert(190 == evaluate_equation(190, std::array{10, 19}));
static_assert(3267 == evaluate_equation(3267, std::array{81, 40, 27}));
static_assert(0 == evaluate_equation(83, std::array{17, 5}));
static_assert(0 == evaluate_equation(156, std::array{15, 6}));
static_assert(0 == evaluate_equation(7290, std::array{6, 8, 6, 15}));
static_assert(0 == evaluate_equation(161011, std::array{16, 10, 13}));
static_assert(0 == evaluate_equation(192, std::array{17, 8, 14}));
static_assert(0 == evaluate_equation(21037, std::array{9, 7, 18, 13}));
static_assert(292 == evaluate_equation(292, std::array{11, 6, 16, 20}));

template <bool words>
int_t solve_case(const std::string& filename) {
  int_t sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [test_value_str, operands_str] =
        aoc::split<std::array<std::string_view, 2>>(line, ':');
    sum +=
        evaluate_equation(aoc::to_number<int_t>(test_value_str),
                          aoc::split<operands_t>(operands_str.substr(1), ' '));
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3749, solve_case<false>("day07.example"));
  AOC_EXPECT_RESULT(5702958180383, solve_case<false>("day07.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day07.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day07.input"));
  AOC_RETURN_CHECK_RESULT();
}
