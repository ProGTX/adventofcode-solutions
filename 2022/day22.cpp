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

template <class neighborhood_t>
struct cell_type;

struct neighborhood : public array_grid<cell_type<neighborhood>*, 3> {};

template <class neighborhood_t>
struct cell_type {
  char value;
  point pos;
  neighborhood_t neighbors;

  template <std::same_as<neighborhood> N = neighborhood_t>
  constexpr cell_type*& get_neighbor(point diff) {
    this->assert_diff(diff);
    return neighbors.at(diff.y + 1, diff.x + 1);
  }
  template <std::same_as<neighborhood> N = neighborhood_t>
  constexpr cell_type* const& get_neighbor(point diff) const {
    this->assert_diff(diff);
    return neighbors.at(diff.y + 1, diff.x + 1);
  }

  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(cell_type const& lhs, char_t value) {
    return lhs.value == static_cast<char>(value);
  }
  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(char_t value, cell_type const& rhs) {
    return rhs.value == static_cast<char>(value);
  }

  friend std::ostream& operator<<(std::ostream& out, cell_type const& cell) {
    out << cell.value;
    return out;
  }

 private:
  constexpr void assert_diff(point diff) {
    (void)diff;
    AOC_ASSERT(std::abs(diff.x * diff.y) != 1, "Not valid to go diagonally");
    AOC_ASSERT(!((diff.x == 0) && (diff.y == 0)), "Not valid to access itself");
  }
};

using cell_t = cell_type<neighborhood>;
using jungle_t = grid<cell_t>;
using row_t = jungle_t::row_t;
using min_max_cell_t = cell_type<min_max_helper>;
using raw_map_t = std::vector<std::vector<min_max_cell_t>>;

enum facing_t : int {
  right = 0,
  down = 1,
  left = 2,
  up = 3,
  NUM_FACING = 4,
};

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

constexpr point get_diff(facing_t facing) {
  switch (facing) {
    case right:
      return {1, 0};
    case down:
      return {0, 1};
    case left:
      return {-1, 0};
    case up:
      return {0, -1};
    default:
      AOC_ASSERT(false, "Facing into an invalid direction");
      return {};
  }
};

std::tuple<point, facing_t> get_position(jungle_t& jungle,
                                         directions_t const& directions,
                                         point pos, facing_t facing) {
  cell_t* current_ptr = nullptr;
  const auto try_move = [&](point const& diff) -> bool {
    auto new_ptr = jungle.at(pos.y, pos.x).get_neighbor(diff);
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
        value.neighbors.min_value.y = vertical_limits.min_value.y;
        value.neighbors.max_value.y = vertical_limits.max_value.y;
      }
    }
  }
}

constexpr inline auto neighbor_diffs = std::invoke([]() {
  std::array<point, NUM_FACING> positions;
  for (int f = 0; f < NUM_FACING; ++f) {
    auto facing = static_cast<facing_t>(f);
    positions[f] = get_diff(facing);
  }
  return positions;
});

void set_neighbors_wrapped(jungle_t& jungle, raw_map_t const& raw_map) {
  std::cout << print_range(neighbor_diffs) << std::endl;
  for (int row = 0; row < jungle.num_rows(); ++row) {
    int column = 0;
    for (; column < jungle.row_length(); ++column) {
      if (is_part_of_cube(raw_map[row][column].value)) {
        break;
      }
    }
    const auto limits = raw_map[row][column].neighbors;
    for (; column < limits.max_value.x; ++column) {
      point pos{column, row};
      const auto current_ptr = &raw_map[row][column];
      const auto section_size =
          current_ptr->neighbors.max_value - current_ptr->neighbors.min_value;
      std::cout << "pos " << pos << std::endl;
      for (const auto diff : neighbor_diffs) {
        auto neighbor_pos =
            current_ptr->neighbors.min_value +
            ((section_size + pos - current_ptr->neighbors.min_value + diff) %
             section_size);
        std::cout << "  neighbor_pos " << neighbor_pos << std::endl;
        jungle.at(row, column).get_neighbor(diff) =
            &jungle.at(neighbor_pos.y, neighbor_pos.x);
      }
    }
  }
}

template <int cube_side>
void set_neighbors_cube(jungle_t& jungle, raw_map_t const& raw_map) {}

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

  const auto parse_map_raw = [&](std::string_view line, int row) {
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
          return min_max_cell_t{value, point{column, row}, horizontal_limits};
        });
    raw_map.push_back(std::move(current_row));
  };

  directions_t directions;
  const auto parse_directions = [&](std::string_view line) {
    facing_t facing = right;
    std::string buffer;
    const auto flush_buffer = [&]() {
      directions.push_back(-std::stoi(buffer));
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

  readfile_op<decltype(get_trimmer_keep_spaces())>(filename, parser);

  grid_size.max_value += point{1, 1};

  // Convert raw map to jungle
  auto row_length = grid_size.max_value.x;
  row_t jungle_row;
  jungle_t jungle;

  for (int row = 0; auto& raw_row : raw_map) {
    for (int column = raw_row.size(); column < row_length; ++column) {
      raw_row.push_back(min_max_cell_t{invalid, point{column, row}, grid_size});
    }
    jungle_row.clear();
    std::ranges::transform(
        raw_row, std::back_inserter(jungle_row),
        [](min_max_cell_t const& cell) {
          return cell_t{cell.value, cell.pos, neighborhood{}};
        });
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
  // AOC_EXPECT_RESULT(97356, (solve_case<false, 50>("day22.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(5031, (solve_case<true, 4>("day22.example")));
  // AOC_EXPECT_RESULT(3229579395609, (solve_case<true, 50>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
