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

constexpr bool is_symbol(char c) { return !aoc::is_number(c) && (c != '.'); }

struct number_t {
  int row;
  int column;
  int size;

  bool operator==(const number_t&) const = default;
};

constexpr std::optional<number_t> find_number(std::string_view line,
                                              int linenum, int init_pos) {
  if (!aoc::is_number(line[init_pos])) {
    return std::nullopt;
  }

  number_t number{linenum - 1, 0, 0};
  int start_pos = init_pos;
  for (; start_pos >= 0; --start_pos) {
    if (!aoc::is_number(line[start_pos])) {
      break;
    }
  }
  // The column is one less than desired after the for loop finishes
  number.column = start_pos + 1;

  int end_pos = init_pos + 1;
  for (; end_pos < line.size(); ++end_pos) {
    if (!aoc::is_number(line[end_pos])) {
      break;
    }
  }
  number.size = end_pos - number.column;

  return number;
}

static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 0));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 1));
static_assert(number_t{0, 0, 3} == *find_number("467..114..", 1, 2));

template <bool check_gears>
int solve_case(const std::string& filename) {
  using number_pair_t = std::pair<number_t, int>;
  std::vector<number_pair_t> all_numbers;
  [[maybe_unused]] std::vector<number_pair_t> current_numbers;

  auto add_number = [&](std::string_view line, int linenum, int pos) {
    auto number = find_number(line, linenum, pos);
    if (!number) {
      return;
    }
    auto it = std::ranges::find(all_numbers, *number, &number_pair_t::first);
    if (it != std::end(all_numbers)) {
      if constexpr (check_gears) {
        if (!ranges::contains(current_numbers, it->first,
                              &number_pair_t::first)) {
          current_numbers.emplace_back(it->first, it->second);
        }
      }
      return;
    }
    auto number_value =
        to_number<int>(line.substr(number->column, number->size));
    all_numbers.emplace_back(*number, number_value);
    if constexpr (check_gears) {
      if (!ranges::contains(current_numbers, *number, &number_pair_t::first)) {
        current_numbers.emplace_back(*number, number_value);
      }
    }
  };

  [[maybe_unused]] std::vector<int> gear_ratios;
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
      if constexpr (check_gears) {
        current_numbers.clear();
      }
      add_number(line_minus2, linenum - 2, pos - 1);
      add_number(line_minus2, linenum - 2, pos);
      add_number(line_minus2, linenum - 2, pos + 1);
      add_number(line_minus1, linenum - 1, pos - 1);
      add_number(line_minus1, linenum - 1, pos + 1);
      add_number(line, linenum, pos - 1);
      add_number(line, linenum, pos);
      add_number(line, linenum, pos + 1);
      if constexpr (check_gears) {
        if (current_numbers.size() == 2) {
          gear_ratios.push_back(current_numbers[0].second *
                                current_numbers[1].second);
        }
      }
    }
    line_minus2 = std::string{line};
    std::swap(line_minus1, line_minus2);
  };

  readfile_op(filename, solver);

  int sum = 0;
  if constexpr (!check_gears) {
    sum = ranges::fold_left(
        all_numbers | std::views::transform(&number_pair_t::second), 0,
        std::plus<>{});
  } else {
    sum = ranges::fold_left(gear_ratios, 0, std::plus<>{});
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4361, solve_case<false>("day03.example"));
  AOC_EXPECT_RESULT(537732, solve_case<false>("day03.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(467835, solve_case<true>("day03.example"));
  AOC_EXPECT_RESULT(84883664, solve_case<true>("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
