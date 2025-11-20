// https://adventofcode.com/2024/day/3

#include "../common/common.h"

#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

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
          left = aoc::to_number<int>(
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
          right = aoc::to_number<int>(
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
      AOC_UNREACHABLE("Invalid condition");
    }
  }
  return sum;
}

constexpr int parse_multiplications_with_enablers(std::string_view line) {
  constexpr auto npos = std::string_view::npos;
  using pos_t = std::remove_const_t<decltype(npos)>;
  constexpr std::string_view do_str = "do()";
  constexpr std::string_view dont_str = "don't()";
  pos_t current_index = 0;
  bool mul_enabled = true;
  int sum = 0;
  while (true) {
    if (mul_enabled) {
      auto index = line.find(dont_str, current_index);
      if (index == npos) {
        auto partial = parse_multiplications(line.substr(current_index));
        sum += partial;
        break;
      } else {
        auto partial = parse_multiplications(
            line.substr(current_index, index - current_index));
        sum += partial;
        mul_enabled = false;
        current_index = index + dont_str.size();
      }
    } else {
      auto index = line.find(do_str, current_index);
      if (index == npos) {
        break;
      } else {
        mul_enabled = true;
        current_index = index + do_str.size();
      }
    }
  }
  return sum;
}

static_assert(2024 == parse_multiplications("mul(44,46)"));
static_assert(33 ==
              parse_multiplications("xmul(2,4)&mul[3,7]!^don't()_mul(5,5)"));
static_assert(8 == parse_multiplications_with_enablers(
                       "xmul(2,4)&mul[3,7]!^don't()_mul(5,5)"));
static_assert(1019406 == parse_multiplications(
                             "[#from())when()/}+%mul(982,733)mul(700,428)}}"));

template <bool enablers>
int solve_case(const std::string& filename) {
  int sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    if constexpr (enablers) {
      sum += parse_multiplications_with_enablers(line);
    } else {
      sum += parse_multiplications(line);
    }
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(161, solve_case<false>("day03.example"));
  AOC_EXPECT_RESULT(174960292, solve_case<false>("day03.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(48, solve_case<true>("day03.example2"));
  AOC_EXPECT_RESULT(-61636489, solve_case<true>("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
