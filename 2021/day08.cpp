// https://adventofcode.com/2021/day/8

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

struct input_line_t {
  using signal_patterns_t = std::array<std::string, 10>;
  using output_t = std::array<std::string, 4>;

  signal_patterns_t signal_patterns;
  output_t output;
};

using input_t = std::vector<input_line_t>;

constexpr inline auto num_digit_lines = std::invoke([] {
  std::array<int, 10> digit_lines;
  digit_lines[0] = 5;
  digit_lines[1] = 2;
  digit_lines[2] = 5;
  digit_lines[3] = 5;
  digit_lines[4] = 4;
  digit_lines[5] = 5;
  digit_lines[6] = 6;
  digit_lines[7] = 3;
  digit_lines[8] = 7;
  digit_lines[9] = 6;
  return digit_lines;
});

int count_easy(input_t const& input) {
  int count = 0;
  constexpr auto easy_digits = std::array{1, 4, 7, 8};
  for (input_line_t const& line : input) {
    count +=
        std::ranges::count_if(line.output, [&](std::string_view digit_str) {
          return std::ranges::any_of(easy_digits, [&](int digit) {
            return digit_str.size() == num_digit_lines[digit];
          });
        });
  }
  return count;
}

template <bool>
int_t solve_case(const std::string& filename) {
  input_t input;

  readfile_op(filename, [&](std::string_view line) {
    auto [signal_patterns_str, output_str] =
        split<std::array<std::string, 2>>(line, '|');
    input.emplace_back(split<typename input_line_t::signal_patterns_t>(
                           signal_patterns_str, ' '),
                       split<typename input_line_t::output_t>(output_str, ' '));
  });

  int_t count = count_easy(input);
  std::cout << filename << " -> " << count << std::endl;
  return count;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(0, (solve_case<false>("day08.example")));
  AOC_EXPECT_RESULT(26, (solve_case<false>("day08.example2")));
  AOC_EXPECT_RESULT(440, (solve_case<false>("day08.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(168, (solve_case<true>("day08.example")));
  // AOC_EXPECT_RESULT(101618069, (solve_case<true>("day08.input")));
  AOC_RETURN_CHECK_RESULT();
}
