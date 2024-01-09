// https://adventofcode.com/2022/day/25

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;
using input_t = std::vector<std::string>;

constexpr int snafu_char_to_int(char value) {
  switch (value) {
    case '-':
      return -1;
    case '=':
      return -2;
    default:
      AOC_ASSERT((value >= '0') && (value <= '2'),
                 "Invalid character provided");
      return static_cast<int>(value - '0');
  }
}

constexpr char int_to_snafu_char(int value) {
  switch (value) {
    case -1:
      return '-';
    case -2:
      return '=';
    default:
      AOC_ASSERT((value >= 0) && (value <= 2), "Invalid integer provided");
      return static_cast<char>(value + '0');
  }
}

template <int base>
constexpr int_t to_decimal(std::string_view snafu) {
  int_t sum = 0;
  for (int_t multiplier = 1;
       int_t value : snafu | std::views::reverse |
                         std::views::transform(&snafu_char_to_int)) {
    sum += value * multiplier;
    multiplier *= base;
  }
  return sum;
}

template <int base>
constexpr std::string sum_snafus(std::string_view lhs, std::string_view rhs) {
  std::string output;
  output.resize(lhs.size() + rhs.size() + 1);
  std::ranges::fill(output, '0');
  std::ranges::view auto lhsr = lhs | std::views::reverse;
  std::ranges::view auto rhsr = rhs | std::views::reverse;
  std::ranges::view auto outr = output | std::views::reverse;
  int min = std::min(lhs.size(), rhs.size());
  int max = std::max(lhs.size(), rhs.size());
  int carry = 0;
  int i = 0;
  for (; i < min; ++i) {
    int sum = snafu_char_to_int(lhsr[i]) + snafu_char_to_int(rhsr[i]) + carry;
    carry = std::lround(static_cast<double>(sum) / base);
    outr[i] = int_to_snafu_char(sum - (carry * base));
  }
  std::ranges::view auto maxr = (lhs.size() == max) ? lhsr : rhsr;
  for (; i < max; ++i) {
    int sum = snafu_char_to_int(maxr[i]) + carry;
    carry = std::lround(static_cast<double>(sum) / base);
    outr[i] = int_to_snafu_char(sum - (carry * base));
  }
  outr[i] = int_to_snafu_char(carry);
  output = std::string{aoc::ltrim(output, "0")};
  return output;
}

template <int base>
constexpr int_t sum_input(input_t const& input) {
  auto snafu_view = input | std::views::transform([](std::string_view snafu) {
                      return to_decimal<base>(snafu);
                    });
  return std::accumulate(std::begin(snafu_view), std::end(snafu_view), 0);
}

template <int base>
constexpr std::string sum_input_as_snafus(input_t const& input) {
  return std::accumulate(std::begin(input) + 1, std::end(input), input[0],
                         [](std::string_view lhs, std::string_view rhs) {
                           return sum_snafus<base>(lhs, rhs);
                         });
}

template <int base>
constexpr std::string to_snafu(const int_t number) {
  int size = aoc::num_digits(number);
  int_t multiplier = aoc::pown<int_t>(base, size - 1);
  do {
    ++size;
    multiplier *= base;
  } while ((number / multiplier) > 0);

  std::string snafu;
  snafu.resize(size);

  int_t current = number;
  for (auto& value : snafu) {
    int multiple = std::lround(static_cast<double>(current) / multiplier);
    value = int_to_snafu_char(multiple);
    current = current - (multiple * multiplier);
    multiplier /= base;
  }
  return std::string{aoc::ltrim(snafu, "0")};
}

template <int base, bool execute_long>
std::string solve_case(std::string const& filename) {
  auto snafu = sum_input_as_snafus<base>(aoc::views::read_lines(filename) |
                                         aoc::ranges::to<input_t>());

  std::cout << filename << " -> " << snafu << std::endl;
  return snafu;
}

#define AOC_EXPECT_SAME_CONVERSION(base, decimal, snafu)                       \
  {                                                                            \
    AOC_EXPECT_RESULT(snafu, to_snafu<base>(decimal));                         \
    static_assert(decimal == to_decimal<base>(snafu));                         \
  }

int main() {
  // Unit tests

  AOC_EXPECT_RESULT("1=21", (sum_snafus<5>(to_snafu<5>(49), to_snafu<5>(37))));

  AOC_EXPECT_SAME_CONVERSION(5, 976, "2=-01");
  AOC_EXPECT_SAME_CONVERSION(5, 1747, "1=-0-2");
  AOC_EXPECT_SAME_CONVERSION(5, 906, "12111");
  AOC_EXPECT_SAME_CONVERSION(5, 198, "2=0=");
  AOC_EXPECT_SAME_CONVERSION(5, 11, "21");
  AOC_EXPECT_SAME_CONVERSION(5, 201, "2=01");
  AOC_EXPECT_SAME_CONVERSION(5, 31, "111");
  AOC_EXPECT_SAME_CONVERSION(5, 1257, "20012");
  AOC_EXPECT_SAME_CONVERSION(5, 32, "112");
  AOC_EXPECT_SAME_CONVERSION(5, 353, "1=-1=");
  AOC_EXPECT_SAME_CONVERSION(5, 107, "1-12");
  AOC_EXPECT_SAME_CONVERSION(5, 7, "12");
  AOC_EXPECT_SAME_CONVERSION(5, 3, "1=");
  AOC_EXPECT_SAME_CONVERSION(5, 37, "122");

  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT("2=-1=0", (solve_case<5, false>("day25.example")));
  AOC_EXPECT_RESULT("2-=12=2-2-2-=0012==2",
                    (solve_case<5, false>("day25.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT("2=-1=0", (solve_case<5, true>("day25.example")));
  // AOC_EXPECT_RESULT("2=-1=0", (solve_case<5, true>("day25.input")));
  AOC_RETURN_CHECK_RESULT();
}
