// https://adventofcode.com/2025/day/3

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

using Batteries = aoc::char_grid<>;

fn parse(String const& filename) -> Batteries {
  return aoc::read_char_grid(filename);
}

struct index_value {
  usize index;
  char value;
};

fn index_of_max(str row) -> usize {
  let max = std::ranges::max(
      row | std::views::enumerate | std::views::transform([](auto&& tuple) {
        let[index, value] = tuple;
        return index_value{.index = static_cast<usize>(index), .value = value};
      }),
      [](index_value const& lhs, index_value const& rhs) {
        if (lhs.value == rhs.value) {
          // We need to ensure that if two elements compare
          // equal, take the first one
          return rhs.index < lhs.index;
        }
        return lhs.value < rhs.value;
      });
  return max.index;
}

template <usize TURN_NUM>
fn solve_case(Batteries const& batteries) -> u64 {
  return aoc::ranges::accumulate(
      Range{0uz, batteries.num_rows()} | std::views::transform([&](usize row) {
        let current = batteries.row_view(row);
        let size = current.size();
        auto indexes = aoc::static_vector<usize, TURN_NUM>{};
        indexes.push_back(
            index_of_max(current.substr(0, size - (TURN_NUM - 1))));
        for (let tail : Range{0uz, (TURN_NUM - 1)} | std::views::reverse) {
          let offset = indexes.back();
          indexes.push_back( //
              offset +
              1 +
              index_of_max(
                  current.substr(offset + 1, size - tail - offset - 1)));
        }
        using pair = aoc::point_type<u64>;
        let result = std::ranges::fold_left(
            indexes | std::views::reverse, pair{0, 1},
            [&](pair acc_pair, usize index) {
              let[acc, multiplier] = acc_pair;
              return pair{
                  acc + aoc::to_number<u64>(current[index]) * multiplier,
                  multiplier * 10};
            });
        return result.x;
      }),
      u64{});
}

int main() {
  std::println("Part 1");
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(357, solve_case<2>(example));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(17166, solve_case<2>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3121910778619, solve_case<12>(example));
  AOC_EXPECT_RESULT(169077317650774, solve_case<12>(input));

  AOC_RETURN_CHECK_RESULT();
}
