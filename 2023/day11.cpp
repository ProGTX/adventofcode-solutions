// https://adventofcode.com/2023/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <string>

using point_t = aoc::point_type<i64>;

inline constexpr char galaxy = '#';
inline constexpr char empty_space = '.';
using space_t = aoc::sparse_grid<char, empty_space, point_t>;

struct Input {
  Vec<String> space_rows;
  Vec<bool> row_populated;
  Vec<bool> column_populated;
};

fn parse(String const& filename) -> Input {
  auto space_rows = aoc::read_lines(filename);

  auto row_populated = //
      space_rows |
      stdv::transform(
          [](String const& row) { return stdr::contains(row, galaxy); }) |
      aoc::collect_vec<bool>();

  let num_columns = space_rows.empty() ? 0uz : space_rows.front().size();
  auto column_populated =
      Range{0uz, num_columns} |
      stdv::transform([&](usize column) {
        return stdr::any_of(space_rows, [column](String const& row) {
          return row[column] == galaxy;
        });
      }) |
      aoc::collect_vec<bool>();

  return Input{std::move(space_rows), std::move(row_populated),
               std::move(column_populated)};
}

template <int factor>
fn expand_space(Input const& input) -> space_t {
  let& space_rows = input.space_rows;
  let& row_populated = input.row_populated;
  let& column_populated = input.column_populated;

  AOC_ASSERT(space_rows.size() > 1, "Cannot construct space from no rows");
  let num_empty_columns = stdr::count(column_populated, false);
  let orig_row_size = space_rows[0].size();
  let num_columns = orig_row_size + num_empty_columns;
  auto space = space_t(space_rows.size(), num_columns);

  auto row = i64{0};
  for (let r : Range{0uz, space_rows.size()}) {
    if (!row_populated[r]) {
      row += factor;
      continue;
    }
    let& current_row = space_rows[r];
    auto column = i64{0};
    for (let c : Range{0uz, orig_row_size}) {
      if (!column_populated[c]) {
        column += factor;
        continue;
      }
      if (current_row[c] == galaxy) {
        space.modify(galaxy, row, column);
      }
      ++column;
    }
    ++row;
  }
  return space;
}

fn sum_distances(space_t const& space) -> i64 {
  return aoc::ranges::accumulate(
      stdv::cartesian_product(space, space) |
          aoc::views::transform_filter([](let& elem) -> Option<i64> {
            let[g1, g2] = elem;
            if (g1.first < g2.first) {
              return distance_manhattan(g1.first, g2.first);
            }
            return None;
          }),
      i64{});
}

template <int factor>
fn solve_case(Input const& input) -> i64 {
  let space = expand_space<factor>(input);
  return sum_distances(space);
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(374, (solve_case<2>(example)));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(10228230, (solve_case<2>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1030, (solve_case<10>(example)));
  AOC_EXPECT_RESULT(8410, (solve_case<100>(example)));
  AOC_EXPECT_RESULT(82000210, (solve_case<1000000>(example)));
  AOC_EXPECT_RESULT(447073334102, (solve_case<1000000>(input)));

  AOC_RETURN_CHECK_RESULT();
}
