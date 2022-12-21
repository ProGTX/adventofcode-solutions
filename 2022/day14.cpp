// https://adventofcode.com/2022/day/14

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

enum special_chars : char {
  empty = '.',
  rock = '#',
  start = '+',
  sand = 'o',
};

template <bool>
void solve_case(const std::string& filename) {
  using rock_line_t = std::array<point, 2>;
  std::vector<rock_line_t> rock_lines;

  point min_value{1'000'000'000, 1'000'000'000};
  point max_value{0, 0};

  const auto update_min_max = [&](const point& p) {
    if (p.x < min_value.x) {
      min_value.x = p.x;
    }
    if (p.y < min_value.y) {
      min_value.y = p.y;
    }
    if (p.x > max_value.x) {
      max_value.x = p.x;
    }
    if (p.y > max_value.y) {
      max_value.y = p.y;
    }
  };

  readfile_op(filename, [&](std::string_view line) {
    // Simplify "->" delimiter to a single char
    std::string transformed_line(line.size(), 0);
    std::ranges::transform(line, std::begin(transformed_line), [](char value) {
      if (value == '>') {
        return ' ';
      }
      return value;
    });
    auto rock_pairs = split<std::vector<std::string>>(transformed_line, '-');
    auto previous = split<point>(rock_pairs[0], ',');
    update_min_max(previous);
    for (const auto& rock_pair_str : rock_pairs | std::views::drop(1)) {
      auto current = split<point>(rock_pair_str, ',');
      update_min_max(current);
      rock_lines.push_back(rock_line_t{previous, current});
      previous = current;
    }
  });

  using cave_map_t = grid<std::vector<char>>;

  point sand_starter{500, 0};
  update_min_max(sand_starter);

  auto cave_dimensions = max_value - min_value + point{1, 1};
  std::cout << "Creating cave map " << cave_dimensions.y << ","
            << cave_dimensions.x << std::endl;
  cave_map_t cave_map{empty, cave_dimensions.y, cave_dimensions.x};

  const auto adjust_coordinates = [&](const point& p) { return p - min_value; };
  const auto modify_cave_map = [&](char value, const point& p) {
    auto coords = adjust_coordinates(p);
    cave_map.modify(value, coords.y, coords.x);
  };

  for (const auto& rock_line : rock_lines) {
    auto diff = rock_line[1] - rock_line[0];
    diff = diff / diff.abs();
    if (diff.x == 0) {
      diff.x = 1;
    }
    if (diff.y == 0) {
      diff.y = 1;
    }
    for (int row = rock_line[0].y; row != rock_line[1].y + diff.y;
         row += diff.y) {
      for (int column = rock_line[0].x; column != rock_line[1].x + diff.x;
           column += diff.x) {
        modify_cave_map(rock, point{column, row});
      }
    }
  }

  modify_cave_map(start, sand_starter);

  cave_map.print_all();

  const auto is_valid_index = [&](int row, int column) {
    return (row >= 0) && (row < cave_map.num_rows()) && (column >= 0) &&
           (column < cave_map.row_length());
  };

  const auto is_empty = [&](int row, int column) {
    return (cave_map.at(row, column) == empty);
  };

  const auto try_insert_grain = [&](int row, int column) {
    bool within_bounds = true;

    // This function returns whether a move was successful
    const auto try_move = [&](int new_row, int new_column) {
      if (!is_valid_index(new_row, new_column)) {
        row = new_row;
        column = new_column;
        // Move was successful, but it's out of bounds
        within_bounds = false;
        return true;
      }
      if (is_empty(new_row, new_column)) {
        row = new_row;
        column = new_column;
        within_bounds = true;
        return true;
      }
      // Move was not successful, try something else
      return false;
    };

    while (within_bounds) {
      // Try to fall down
      if (try_move(row + 1, column)) {
        continue;
      }
      // Try to fall on the left
      if (try_move(row + 1, column - 1)) {
        continue;
      }
      // Try to fall on the right
      if (try_move(row + 1, column + 1)) {
        continue;
      }
      within_bounds = false;
    }
    if (is_valid_index(row, column) && is_empty(row, column)) {
      cave_map.modify(sand, row, column);
      return true;
    } else {
      return false;
    }
  };

  // Simulate sand falling
  int num_grains = 0;
  const auto adjusted_start = adjust_coordinates(sand_starter);
  const auto index_increase = cave_map.row_length();
  while (true) {
    bool sand_success = false;
    auto previous_index =
        cave_map.linear_index(adjusted_start.y, adjusted_start.x);
    // First go down from the source to the lowest level
    for (auto linear_index = previous_index + index_increase;
         (linear_index < cave_map.size()); linear_index += index_increase) {
      auto value = cave_map.data()[linear_index];
      if (value == start) {
        AOC_ASSERT(false, "This condition shouldn't be reached");
      }
      if ((value == rock) || (value == sand)) {
        // Found solid ground, try to insert it above
        int row = previous_index / index_increase;
        int column = previous_index % index_increase;
        if (try_insert_grain(row, column)) {
          sand_success = true;
          break;
        } else {
          sand_success = false;
          break;
        }
      }
      previous_index = linear_index;
    }
    if (!sand_success) {
      break;
    }
    ++num_grains;
  }

  cave_map.print_all();

  std::cout << filename << " -> " << num_grains << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day14.example");
  solve_case<false>("day14.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day14.example");
  // solve_case<true>("day14.input");
}
