// https://adventofcode.com/2024/day/7

#include "../common/common.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::uint64_t;

// The individual operands can be 32-bit ints
using operands_t = aoc::static_vector<int, 12>;

template <bool concat>
constexpr int get_operation_id(unsigned evaluation_id, int bit_pos) {
  return (evaluation_id >> ((concat ? 2 : 1) * bit_pos)) & (concat ? 3 : 1);
}

constexpr auto bit_pos_view(unsigned max_operators) {
  return std::views::iota(0, static_cast<int>(max_operators)) |
         std::views::reverse;
}

template <bool concat>
constexpr int_t evaluate_equation(const int_t test_value,
                                  std::span<const int> operands) {
  const unsigned max_operators = operands.size() - 1;
  const unsigned max_evaluations = 1 << ((concat ? 2 : 1) * max_operators);
  for (unsigned id = 0; id < max_evaluations; ++id) {
    operands_t operation_ids;
    operation_ids.resize(max_operators);
    std::conditional_t<concat, bool, const bool> skip_evaluation = false;
    for (int bit_pos : bit_pos_view(max_operators)) {
      operation_ids[bit_pos] = get_operation_id<concat>(id, bit_pos);
      if constexpr (concat) {
        if (operation_ids[bit_pos] == 3) {
          // We only have 3 possible operations, but we use 2 bits to represent
          // them, so we need to skip every time we get the 4th operation
          skip_evaluation = true;
          break;
        }
      }
    }
    if (skip_evaluation) {
      continue;
    }
    int_t sum = operands[0];
    for (int bit_pos : bit_pos_view(max_operators)) {
      auto current_num = operands[max_operators - bit_pos];
      auto operation_id = operation_ids[bit_pos];
      if (operation_id == 0) {
        sum += current_num;
      } else if (operation_id == 1) {
        sum *= current_num;
      } else if constexpr (concat) {
        if (operation_id == 2) {
          sum = aoc::concat_numbers(sum, static_cast<unsigned>(current_num));
        } else {
          AOC_ASSERT(false, "Invalid operation ID");
        }
      }
    }
    if (sum == test_value) {
      return sum;
    }
  }
  return 0;
}

static_assert(190 == evaluate_equation<false>(190, std::array{10, 19}));
static_assert(3267 == evaluate_equation<false>(3267, std::array{81, 40, 27}));
static_assert(0 == evaluate_equation<false>(83, std::array{17, 5}));
static_assert(0 == evaluate_equation<false>(156, std::array{15, 6}));
static_assert(0 == evaluate_equation<false>(7290, std::array{6, 8, 6, 15}));
static_assert(0 == evaluate_equation<false>(161011, std::array{16, 10, 13}));
static_assert(0 == evaluate_equation<false>(192, std::array{17, 8, 14}));
static_assert(0 == evaluate_equation<false>(21037, std::array{9, 7, 18, 13}));
static_assert(292 == evaluate_equation<false>(292, std::array{11, 6, 16, 20}));

static_assert(190 == evaluate_equation<true>(190, std::array{10, 19}));
static_assert(3267 == evaluate_equation<true>(3267, std::array{81, 40, 27}));
static_assert(0 == evaluate_equation<true>(83, std::array{17, 5}));
static_assert(156 == evaluate_equation<true>(156, std::array{15, 6}));
static_assert(7290 == evaluate_equation<true>(7290, std::array{6, 8, 6, 15}));
static_assert(0 == evaluate_equation<true>(161011, std::array{16, 10, 13}));
static_assert(192 == evaluate_equation<true>(192, std::array{17, 8, 14}));
static_assert(0 == evaluate_equation<true>(21037, std::array{9, 7, 18, 13}));
static_assert(292 == evaluate_equation<true>(292, std::array{11, 6, 16, 20}));

static_assert(3125401 ==
              evaluate_equation<true>(3125401, std::array{62, 456, 52, 5, 1}));

template <bool concat>
int_t solve_case(const std::string& filename) {
  int_t sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [test_value_str, operands_str] = aoc::split_once(line, ':');
    auto test_value = aoc::to_number<int_t>(test_value_str);
    auto operands = aoc::split<operands_t>(operands_str.substr(1), ' ');
    sum += evaluate_equation<concat>(test_value, operands);
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3749, solve_case<false>("day07.example"));
  AOC_EXPECT_RESULT(5702958180383, solve_case<false>("day07.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(11387, solve_case<true>("day07.example"));
  AOC_EXPECT_RESULT(92612386119138, solve_case<true>("day07.input"));
  AOC_RETURN_CHECK_RESULT();
}
