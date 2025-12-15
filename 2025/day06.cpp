// https://adventofcode.com/2025/day/6

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using CharGrid = aoc::char_grid<>;

auto parse(String const& filename) -> CharGrid {
  return aoc::read_char_grid(filename, aoc::keep_spaces{});
}

fn solve_case1(CharGrid const& char_grid) -> u64 {
  let numbers = [&] {
    auto num_columns = 0uz;
    auto data = Vec<u64>{};
    for (let row_id : Range{0uz, char_grid.num_rows() - 1}) {
      let line = char_grid.row_view(row_id);
      aoc::ranges::extend(data, aoc::split_sstream<u64>(line));
      if (num_columns == 0) {
        num_columns = data.size();
      }
    }
    return aoc::grid<u64>(data, char_grid.num_rows() - 1, num_columns);
  }();
  let operations =
      char_grid.row_view(char_grid.num_rows() - 1) |
      std::views::filter([](char c) { return (c == '+') || (c == '*'); }) |
      aoc::ranges::to<String>();
  return aoc::ranges::accumulate( //
      Range{0uz, numbers.num_columns()} |
          std::views::transform([&](let column_id) {
            let add = (operations[column_id] == '+');
            let init = add ? u64{} : 1;
            return std::ranges::fold_left( //
                numbers.column_view(column_id), init,
                [&](u64 acc, u64 current) {
                  return add ? (acc + current) : (acc * current);
                });
          }),
      u64{});
}

fn solve_case2(CharGrid const& char_grid) -> u64 {
  auto result = u64{};
  auto current = Vec<u64>{};
  constexpr let EMPTY = ' ';
  for (let column_id :
       Range{0uz, char_grid.num_columns()} | std::views::reverse) {
    auto number = 0;
    for (let value_char : char_grid.column_view(column_id) |
                              std::views::take(char_grid.num_rows() - 1)) {
      if (value_char == EMPTY) {
        continue;
      }
      number = number * 10 + aoc::to_number<u64>(value_char);
    }
    if (number == 0) {
      continue;
    }
    current.push_back(number);
    let op = char_grid.at(char_grid.num_rows() - 1, column_id);
    if (op == EMPTY) {
      continue;
    }
    let add = (op == '+');
    let init = add ? u64{} : 1;
    result += std::ranges::fold_left( //
        current, init, [&](u64 acc, u64 current) {
          return add ? (acc + current) : (acc * current);
        });
    current.clear();
  }
  return result;
}

int main() {
  std::println("Part 1");
  let example = parse("day06.example");
  AOC_EXPECT_RESULT(4277556, solve_case1(example));
  let input = parse("day06.input");
  AOC_EXPECT_RESULT(5322004718681, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3263827, solve_case2(example));
  AOC_EXPECT_RESULT(9876636978528, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
