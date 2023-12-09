// https://adventofcode.com/2023/day/3

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

constexpr bool is_number(char c) { return (c >= '0') && (c <= '9'); }
constexpr bool is_symbol(char c) { return !is_number(c) && (c != '.'); }

struct number_t {
  int row;
  int column;
  int size;

  bool operator==(const number_t&) const = default;
};

constexpr std::optional<number_t> find_number(std::string_view line,
                                              int linenum, int init_pos) {
  if (!is_number(line[init_pos])) {
    return std::nullopt;
  }

  number_t number{linenum - 1, 0, 0};
  int start_pos = init_pos;
  for (; start_pos >= 0; --start_pos) {
    if (!is_number(line[start_pos])) {
      break;
    }
  }
  // The column is one less than desired after the for loop finishes
  number.column = start_pos + 1;

  int end_pos = init_pos + 1;
  for (; end_pos < line.size(); ++end_pos) {
    if (!is_number(line[end_pos])) {
      break;
    }
  }
  number.size = end_pos - number.column;

  return number;
}

static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 0));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 1));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 2));

template <bool>
int solve_case(const std::string& filename) {
  using number_pair_t = std::pair<number_t, int>;
  std::vector<number_pair_t> numbers;
  auto add_number = [&](std::string_view line, int linenum, int pos) {
    auto number = find_number(line, linenum, pos);
    if (!number) {
      return;
    }
    if (contains(numbers, *number, &number_pair_t::first)) {
      return;
    }
    numbers.emplace_back(
        *number, to_number<int>(line.substr(number->column, number->size)));
  };

  std::string line_minus2;
  std::string line_minus1;
  auto solver = [&](std::string_view line, int linenum) {
    if (linenum < 3) [[unlikely]] {
      if (linenum == 1) {
        line_minus2 = std::string{line};
      } else if (linenum == 2) {
        line_minus1 = std::string{line};
      }
      return;
    }

    // Note that we're cheating a bit here based on the knowledge
    // that the edges don't contain a symbol, so we can get away with
    // just checking the inner values and not doing any bounds checks
    for (int pos = 1; pos < line_minus1.size() - 1; ++pos) {
      auto c = line_minus1[pos];
      if (!is_symbol(c)) {
        continue;
      }
      add_number(line_minus2, linenum - 2, pos - 1);
      add_number(line_minus2, linenum - 2, pos);
      add_number(line_minus2, linenum - 2, pos + 1);
      add_number(line_minus1, linenum - 1, pos - 1);
      add_number(line_minus1, linenum - 1, pos + 1);
      add_number(line, linenum, pos - 1);
      add_number(line, linenum, pos);
      add_number(line, linenum, pos + 1);
    }
    line_minus2 = std::string{line};
    std::swap(line_minus1, line_minus2);
  };

  readfile_op(filename, solver);

  int sum = 0;
  for (auto&& [number, value] : numbers) {
    sum += value;
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4361, solve_case<false>("day03.example"));
  AOC_EXPECT_RESULT(537732, solve_case<false>("day03.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(2286, solve_case<true>("day03.example"));
  // AOC_EXPECT_RESULT(86036, solve_case<true>("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
