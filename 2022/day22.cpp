// https://adventofcode.com/2022/day/22

#include "../common/assert.h"
#include "../common/common.h"
#include "../common/grid.h"
#include "../common/print.h"

#include <algorithm>
#include <array>
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

enum tile_type_t : char {
  invalid = ' ',
  empty = '.',
  wall = '#',
};

struct corner_t {
  point pos;
  grid_neighbors<corner_t> neighbors;

  constexpr corner_t(point pos_ = {}) : pos{pos_} {}

  friend constexpr bool operator==(corner_t const& lhs, point const& pos) {
    return lhs.pos == pos;
  }
  friend constexpr bool operator==(point const& pos, corner_t const& rhs) {
    return pos == rhs.pos;
  }
};

struct cell_t {
  char value;
  point pos;
  min_max_helper limits;
  grid_neighbors<cell_t> neighbors;

  cell_t(char value_ = invalid, point pos_ = {0, 0},
         min_max_helper limits_ = {})
      : value{value_}, pos{pos_}, limits{limits_} {}

  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(cell_t const& lhs, char_t value) {
    return lhs.value == static_cast<char>(value);
  }
  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(char_t value, cell_t const& rhs) {
    return rhs.value == static_cast<char>(value);
  }

  friend std::ostream& operator<<(std::ostream& out, cell_t const& cell) {
    out << cell.value;
    return out;
  }
};

using jungle_t = grid<cell_t>;
using row_t = jungle_t::row_t;
using raw_map_t = std::vector<std::vector<cell_t>>;

// Store direction change as a non-negative number,
// store number of steps as a negative number
using directions_t = std::vector<int>;

void print_jungle(jungle_t const& jungle) {
  //
  jungle.print_all();
}

constexpr bool is_part_of_cube(char value) {
  return (value == empty) || (value == wall);
}
constexpr bool can_move_to(char value) {
  return (value != invalid) && (value != wall);
}

std::tuple<point, facing_t> get_position(jungle_t& jungle,
                                         directions_t const& directions,
                                         point pos, facing_t facing) {
  cell_t* current_ptr = nullptr;
  const auto try_move = [&](point const& diff) -> bool {
    auto new_ptr = jungle.at(pos.y, pos.x).neighbors.get(diff);
    AOC_ASSERT(new_ptr != nullptr, "Retrieved invalid neighbor");
    if (new_ptr == current_ptr) {
      return false;
    }
    if (!can_move_to(new_ptr->value)) {
      return false;
    }
    const char facing_char = std::invoke([&]() {
      switch (facing) {
        case right:
          return '>';
        case down:
          return 'v';
        case left:
          return '<';
        case up:
          return '^';
        default:
          AOC_ASSERT(false, "Facing into an invalid direction");
          return '~';
      }
    });
    current_ptr->value = facing_char;
    // print_jungle(jungle);
    current_ptr = new_ptr;
    pos = new_ptr->pos;
    return true;
  };
  for (auto direction : directions) {
    if (direction >= 0) {
      facing = static_cast<facing_t>(direction);
      continue;
    }
    int num_steps = -direction;
    const auto diff = get_diff(facing);
    current_ptr = &jungle.at(pos.y, pos.x);
    for (int i = 0; i < num_steps; ++i) {
      if (!try_move(diff)) {
        break;
      }
    }
  }
  return {pos, facing};
}

void adjust_top_to_bottom(raw_map_t& raw_map) {
  const auto row_length = raw_map[0].size();
  const auto num_rows = raw_map.size();
  for (int column = 0; column < row_length; ++column) {
    min_max_helper vertical_limits;
    for (int row = 0; row < num_rows; ++row) {
      auto const& value = raw_map[row][column];
      if (is_part_of_cube(value.value)) {
        vertical_limits.update({0, row});
      }
    }
    vertical_limits.max_value.y += 1;
    for (int row = 0; row < num_rows; ++row) {
      auto& value = raw_map[row][column];
      if (is_part_of_cube(value.value)) {
        value.limits.min_value.y = vertical_limits.min_value.y;
        value.limits.max_value.y = vertical_limits.max_value.y;
      }
    }
  }
}

void set_neighbors_wrapped(jungle_t& jungle, raw_map_t const& raw_map) {
  for (int row = 0; row < jungle.num_rows(); ++row) {
    int column = 0;
    for (; column < jungle.row_length(); ++column) {
      if (is_part_of_cube(raw_map[row][column].value)) {
        break;
      }
    }
    const auto limits = raw_map[row][column].limits;
    for (; column < limits.max_value.x; ++column) {
      point pos{column, row};
      const auto current_ptr = &raw_map[row][column];
      const auto section_size =
          current_ptr->limits.max_value - current_ptr->limits.min_value;
      for (const auto diff : basic_neighbor_diffs) {
        auto neighbor_pos =
            current_ptr->limits.min_value +
            ((section_size + pos - current_ptr->limits.min_value + diff) %
             section_size);
        jungle.at(row, column)
            .neighbors.set(diff, &jungle.at(neighbor_pos.y, neighbor_pos.x));
      }
    }
  }
}

template <int cube_side>
void set_neighbors_cube(jungle_t& jungle, raw_map_t const& raw_map) {
  static constexpr int max_sides = 4;
  const auto num_sides = std::invoke([&]() -> point {
    auto num_sides_horizontal = jungle.row_length() / cube_side;
    if (num_sides_horizontal == 3) {
      return {3, max_sides};
    } else if (num_sides_horizontal == max_sides) {
      return {max_sides, 3};
    } else {
      AOC_ASSERT(false, "Invalid cube");
      return {};
    }
  });

  array_grid<corner_t, max_sides> cube_corners;
  constexpr corner_t invalid_corner = point{-1, -1};
  for (int row_side = 0; row_side < num_sides.y; ++row_side) {
    for (int column_side = 0; column_side < num_sides.x; ++column_side) {
      corner_t corner = point{column_side * cube_side, row_side * cube_side};
      if ((corner.pos.x >= jungle.row_length()) ||
          (corner.pos.y >= jungle.num_rows())) {
        corner = invalid_corner;
      } else if (!is_part_of_cube(
                     jungle.at(corner.pos.y, corner.pos.x).value)) {
        corner = invalid_corner;
      }
      cube_corners.at(row_side, column_side) = corner;
    }
    for (int column_side = num_sides.x; column_side < max_sides;
         ++column_side) {
      cube_corners.at(row_side, column_side) = invalid_corner;
    }
  }
  for (int row_side = num_sides.y; row_side < max_sides; ++row_side) {
    for (int column_side = 0; column_side < max_sides; ++column_side) {
      cube_corners.at(row_side, column_side) = invalid_corner;
    }
  }
  set_standard_neighbors(jungle);
}

template <bool is_cube, int cube_side>
void set_neighbors(jungle_t& jungle, raw_map_t& raw_map) {
  if constexpr (!is_cube) {
    adjust_top_to_bottom(raw_map);
  }
  if constexpr (!is_cube) {
    set_neighbors_wrapped(jungle, raw_map);
  } else {
    set_neighbors_cube<cube_side>(jungle, raw_map);
  }
}

template <bool is_cube, int cube_side>
int solve_case(std::string const& filename) {
  std::string new_line;
  raw_map_t raw_map;
  typename raw_map_t::value_type current_row;

  min_max_helper grid_size;

  const auto parse_map_raw = [&](std::string_view line, int linenum) {
    int row = linenum - 1;
    min_max_helper horizontal_limits;
    int column = -1;
    new_line.clear();
    std::ranges::transform(line, std::back_inserter(new_line),
                           [&](const char value) {
                             ++column;
                             if (is_part_of_cube(value)) {
                               grid_size.update({column, row});
                               horizontal_limits.update({column, 0});
                               return value;
                             } else {
                               return static_cast<char>(invalid);
                             }
                           });
    // Needs to point past the end
    // We'll update vertical limits later
    horizontal_limits.max_value.x += 1;

    column = -1;
    current_row.clear();
    std::ranges::transform(
        new_line, std::back_inserter(current_row), [&](const char value) {
          ++column;
          return cell_t{value, point{column, row}, horizontal_limits};
        });
    raw_map.push_back(std::move(current_row));
  };

  directions_t directions;
  const auto parse_directions = [&](std::string_view line) {
    facing_t facing = right;
    std::string buffer;
    const auto flush_buffer = [&]() {
      directions.push_back(-to_number<int>(buffer));
      buffer.clear();
    };
    for (const char value : line) {
      if (value == 'R') {
        flush_buffer();
        facing = static_cast<facing_t>((facing + 1) % 4);
        directions.push_back(facing);
      } else if (value == 'L') {
        flush_buffer();
        facing = static_cast<facing_t>((4 + facing - 1) % 4);
        directions.push_back(facing);
      } else {
        buffer.push_back(value);
      }
    }
    flush_buffer();
  };

  bool parsing_map = true;
  bool stop_parsing = false;
  const auto parser = [&](std::string_view line, int linenum) {
    if (stop_parsing) {
      return;
    }
    if (line.empty()) {
      parsing_map = false;
      return;
    }
    if (parsing_map) {
      parse_map_raw(line, linenum);
    } else {
      parse_directions(line);
      stop_parsing = false;
    }
  };

  readfile_op<trimmer_keep_spaces<>>(filename, parser);

  grid_size.max_value += point{1, 1};

  // Convert raw map to jungle
  auto row_length = grid_size.max_value.x;
  row_t jungle_row;
  jungle_t jungle;

  for (int row = 0; auto& raw_row : raw_map) {
    for (int column = raw_row.size(); column < row_length; ++column) {
      raw_row.push_back(cell_t{invalid, point{column, row}, grid_size});
    }
    jungle_row = raw_row;
    jungle.add_row(jungle_row);
    ++row;
  }

  set_neighbors<is_cube, cube_side>(jungle, raw_map);

  point start;
  for (int column = 0; column < jungle.row_length(); ++column) {
    if (jungle.at(0, column) == empty) {
      start.x = column;
      break;
    }
  }

  auto [pos, facing] = get_position(jungle, directions, start, right);

  // Indexing starts at 1 for the puzzle
  pos += {1, 1};
  auto password = 1000 * pos.y + 4 * pos.x + facing;
  std::cout << filename << " -> " << password << std::endl;
  return password;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6032, (solve_case<false, 4>("day22.example")));
  AOC_EXPECT_RESULT(97356, (solve_case<false, 50>("day22.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(5031, (solve_case<true, 4>("day22.example")));
  // AOC_EXPECT_RESULT(3229579395609, (solve_case<true, 50>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
