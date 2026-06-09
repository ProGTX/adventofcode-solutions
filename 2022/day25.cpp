// https://adventofcode.com/2022/day/25

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <cmath>
#include <print>
#include <ranges>

fn parse(String const& filename) -> Vec<String> {
  return aoc::read_lines(filename);
}

fn snafu_char_to_int(char value) -> int {
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

fn int_to_snafu_char(int value) -> char {
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
fn to_decimal(str snafu) -> i64 {
  i64 total = 0;
  for (i64 multiplier = 1;
       i64 digit :
       snafu | stdv::reverse | stdv::transform(&snafu_char_to_int)) {
    total += digit * multiplier;
    multiplier *= base;
  }
  return total;
}

template <int base>
fn sum_snafus(str lhs, str rhs) -> String {
  auto output = String{};
  output.resize(lhs.size() + rhs.size() + 1);
  stdr::fill(output, '0');
  let lhs_reversed = lhs | stdv::reverse;
  let rhs_reversed = rhs | stdv::reverse;
  auto output_reversed = output | stdv::reverse;
  let min_len = static_cast<int>(std::min(lhs.size(), rhs.size()));
  let max_len = static_cast<int>(std::max(lhs.size(), rhs.size()));
  int carry = 0;
  int idx = 0;
  for (; idx < min_len; ++idx) {
    int digit_sum = snafu_char_to_int(lhs_reversed[idx]) +
                    snafu_char_to_int(rhs_reversed[idx]) +
                    carry;
    carry =
        static_cast<int>(std::lround(static_cast<double>(digit_sum) / base));
    output_reversed[idx] = int_to_snafu_char(digit_sum - (carry * base));
  }
  let longer_reversed =
      (lhs.size() >= rhs.size()) ? lhs_reversed : rhs_reversed;
  for (; idx < max_len; ++idx) {
    int digit_sum = snafu_char_to_int(longer_reversed[idx]) + carry;
    carry =
        static_cast<int>(std::lround(static_cast<double>(digit_sum) / base));
    output_reversed[idx] = int_to_snafu_char(digit_sum - (carry * base));
  }
  output_reversed[idx] = int_to_snafu_char(carry);
  output = String{aoc::ltrim(output, "0")};
  return output;
}

template <int base>
fn to_snafu(const i64 number) -> String {
  int size = aoc::num_digits(number);
  i64 multiplier = aoc::pown<i64>(base, size - 1);
  do {
    ++size;
    multiplier *= base;
  } while ((number / multiplier) > 0);

  auto snafu = String{};
  snafu.resize(size);

  i64 remaining = number;
  for (auto& output_char : snafu) {
    let digit = static_cast<int>(
        std::lround(static_cast<double>(remaining) / multiplier));
    output_char = int_to_snafu_char(digit);
    remaining -= digit * multiplier;
    multiplier /= base;
  }
  return String{aoc::ltrim(snafu, "0")};
}

fn solve(Vec<String> const& input) -> String {
  return stdr::fold_left(input | stdv::drop(1), input[0], sum_snafus<5>);
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

  std::println("Part 1");
  let example = parse("day25.example");
  AOC_EXPECT_RESULT("2=-1=0", solve(example));
  let input = parse("day25.input");
  AOC_EXPECT_RESULT("2-=12=2-2-2-=0012==2", solve(input));

  AOC_RETURN_CHECK_RESULT();
}
