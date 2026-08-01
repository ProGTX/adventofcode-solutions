// https://adventofcode.com/2024/day/7

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::uint64_t;

// The individual operands can be 32-bit ints
using operands_t = aoc::static_vector<int, 12>;

template <bool concat>
constexpr int_t evaluate_equation(const int_t test_value,
                                  std::span<const int> operands) {
  const unsigned max_operators = operands.size() - 1;
  // 2 possible operators (+, *), or 3 with concatenation (+, *, ||)
  // Enumerate operator assignments directly in that base,
  // rather than in binary/base-4 with a step to skip invalid combinations,
  // so every id enumerated below is valid,
  // none are ever generated only to be discarded
  constexpr unsigned base = concat ? 3 : 2;
  unsigned max_evaluations = 1;
  for (unsigned i = 0; i < max_operators; ++i) {
    max_evaluations *= base;
  }
  for (unsigned id = 0; id < max_evaluations; ++id) {
    int_t sum = operands[0];
    unsigned code = id;
    for (unsigned i = 1; i <= max_operators; ++i) {
      auto current_num = operands[i];
      auto operation_id = code % base;
      code /= base;
      if (operation_id == 0) {
        sum += current_num;
      } else if (operation_id == 1) {
        sum *= current_num;
      } else if constexpr (concat) {
        sum = aoc::concat_numbers(sum, static_cast<unsigned>(current_num));
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

struct equation_t {
  int_t test_value;
  operands_t operands;
};

using equations_t = std::vector<equation_t>;

equations_t parse(const std::string& filename) {
  equations_t equations;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [test_value_str, operands_str] = aoc::split_once(line, ':');
    equations.emplace_back(aoc::to_number<int_t>(test_value_str),
                           aoc::split<operands_t>(operands_str.substr(1), ' '));
  }

  return equations;
}

template <bool concat>
int_t solve_case(const equations_t& equations) {
  int_t sum = 0;

  for (const auto& [test_value, operands] : equations) {
    sum += evaluate_equation<concat>(test_value, operands);
  }

  return sum;
}

int main() {
  std::println("Part 1");
  const auto example = parse("day07.example");
  AOC_EXPECT_RESULT(3749, solve_case<false>(example));
  const auto input = parse("day07.input");
  AOC_EXPECT_RESULT(5702958180383, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(11387, solve_case<true>(example));
  AOC_EXPECT_RESULT(92612386119138, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
