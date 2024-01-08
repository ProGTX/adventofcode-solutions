// https://adventofcode.com/2023/day/11

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using int_t = std::int64_t;
using point_t = aoc::point_type<int_t>;

inline constexpr char galaxy = '#';
inline constexpr char empty_space = '.';
using space_t = aoc::sparse_grid<char, empty_space, point_t>;

template <int factor>
space_t expand_space(const std::vector<std::string>& space_rows,
                     const std::vector<bool>& row_populated,
                     const std::vector<bool>& column_populated) {
  AOC_ASSERT(space_rows.size() > 1, "Cannot construct space from no rows");
  const auto num_empty_columns = std::ranges::count(column_populated, false);
  const auto orig_row_size = space_rows[0].size();
  const auto num_columns = orig_row_size + num_empty_columns;
  auto space = space_t(space_rows.size(), num_columns);
  int_t row = 0;
  for (int r = 0; r < space_rows.size(); ++r, ++row) {
    if (!row_populated[r]) {
      row += factor - 1;
      continue;
    }
    const auto& current_row = space_rows[r];
    int_t column = 0;
    for (int c = 0; c < orig_row_size; ++c, ++column) {
      if (!column_populated[c]) {
        column += factor - 1;
        continue;
      } else if (current_row[c] != galaxy) {
        continue;
      }
      space.modify(galaxy, row, column);
    }
  }
  return space;
}

int_t sum_distances(const space_t& space) {
  int_t sum = 0;
  for (auto it1 = std::begin(space); it1 != std::end(space); ++it1) {
    auto it2 = it1;
    for (++it2; it2 != std::end(space); ++it2) {
      sum += distance_manhattan(it1->first, it2->first);
    }
  }
  return sum;
}

template <int factor>
int_t solve_case(const std::string& filename) {
  std::vector<std::string> space_rows;
  std::string empty_row;
  std::vector<bool> column_populated;
  std::vector<bool> row_populated;

  auto read_values = [&](std::string&& line) {
    const auto size = line.size();
    if (empty_row.empty()) {
      empty_row.reserve(size);
      column_populated.reserve(size);
      std::ranges::fill_n(std::back_inserter(empty_row), size, empty_space);
      std::ranges::fill_n(std::back_inserter(column_populated), size, false);
    }
    row_populated.push_back(line != empty_row);
    for (int i = 0; i < size; ++i) {
      column_populated[i] = column_populated[i] || (line[i] != empty_space);
    }
    space_rows.push_back(std::move(line));
  };
  aoc::readfile_op(filename, read_values);

  auto space =
      expand_space<factor>(space_rows, row_populated, column_populated);

  int_t sum = sum_distances(space);
  std::cout << filename << " (" << factor << ") -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(374, (solve_case<2>("day11.example")));
  AOC_EXPECT_RESULT(10228230, (solve_case<2>("day11.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1030, (solve_case<10>("day11.example")));
  AOC_EXPECT_RESULT(8410, (solve_case<100>("day11.example")));
  AOC_EXPECT_RESULT(82000210, (solve_case<1000000>("day11.example")));
  AOC_EXPECT_RESULT(447073334102, (solve_case<1000000>("day11.input")));
  AOC_RETURN_CHECK_RESULT();
}
