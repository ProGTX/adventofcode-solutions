// https://adventofcode.com/2023/day/11

#include "../common/common.h"
#include "../common/grid.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

inline constexpr char galaxy = '#';
inline constexpr char empty_space = '.';
using space_t = sparse_grid<char, empty_space>;

space_t expand_columns(const std::vector<std::string>& space_rows,
                       const std::vector<bool>& column_full) {
  AOC_ASSERT(space_rows.size() > 1, "Cannot construct space from no rows");
  const auto num_empty_columns = std::ranges::count(column_full, false);
  const auto orig_row_size = space_rows[0].size();
  const auto num_columns = orig_row_size + num_empty_columns;
  auto space = space_t(space_rows.size(), num_columns);
  int row = 0;
  for (const auto& current_row : space_rows) {
    int column = 0;
    for (int c = 0; c < orig_row_size; ++c, ++column) {
      if (!column_full[c]) {
        ++column;
        continue;
      } else if (current_row[c] != galaxy) {
        continue;
      }
      space.modify(galaxy, row, column);
    }
    ++row;
  }
  return space;
}

int sum_distances(const space_t& space) {
  int sum = 0;
  for (auto it1 = std::begin(space); it1 != std::end(space); ++it1) {
    auto it2 = it1;
    for (++it2; it2 != std::end(space); ++it2) {
      sum += point::distance_manhattan(it1->first, it2->first);
    }
  }
  return sum;
}

template <bool>
int solve_case(const std::string& filename) {
  std::vector<std::string> space_rows;
  std::vector<bool> column_full;
  std::string empty_row;

  auto read_values = [&](std::string&& line) {
    const auto size = line.size();
    if (empty_row.empty()) {
      empty_row.reserve(size);
      column_full.reserve(size);
      std::ranges::fill_n(std::back_inserter(empty_row), size, empty_space);
      std::ranges::fill_n(std::back_inserter(column_full), size, false);
    }
    if (line == empty_row) {
      // Extra insertion to expand the row
      space_rows.push_back(empty_row);
    }
    for (int i = 0; i < size; ++i) {
      column_full[i] = column_full[i] || (line[i] != empty_space);
    }
    space_rows.push_back(std::move(line));
  };
  readfile_op(filename, read_values);

  auto space = expand_columns(space_rows, column_full);

  int sum = sum_distances(space);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(374, (solve_case<false>("day11.example")));
  AOC_EXPECT_RESULT(10228230, (solve_case<false>("day11.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(2, (solve_case<true>("day11.example")));
  // AOC_EXPECT_RESULT(977, (solve_case<true>("day11.input")));
  AOC_RETURN_CHECK_RESULT();
}
