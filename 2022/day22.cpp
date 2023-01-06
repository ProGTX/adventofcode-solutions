// https://adventofcode.com/2022/day/22

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

struct cell_t {
  char value;
  min_max_helper limits;

  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(const cell_t& lhs, char_t value) {
    return lhs.value == static_cast<char>(value);
  }
  template <std::convertible_to<char> char_t>
  friend constexpr bool operator==(char_t value, const cell_t& rhs) {
    return rhs.value == static_cast<char>(value);
  }

  friend std::ostream& operator<<(std::ostream& out, const cell_t& cell) {
    out << cell.value;
    return out;
  }
};

using jungle_t = grid<cell_t>;
using row_t = jungle_t::row_t;

enum facing_t : int {
  right = 0,
  down = 1,
  left = 2,
  up = 3,
};

// Store direction change as a non-negative number,
// store number of steps as a negative number
using directions_t = std::vector<int>;

void print_jungle(const jungle_t& jungle) {
  //
  jungle.print_all();
}

std::tuple<point, facing_t> get_position(jungle_t& jungle,
                                         const directions_t& directions,
                                         point pos, facing_t facing) {
  const auto get_diff = [&]() -> point {
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
  cell_t* current_ptr = nullptr;
  const auto try_move = [&](const point& diff) -> bool {
    auto section_size =
        current_ptr->limits.max_value - current_ptr->limits.min_value;
    auto new_pos =
        current_ptr->limits.min_value +
        ((section_size + pos - current_ptr->limits.min_value + diff) %
         section_size);
    if (new_pos == pos) {
      return false;
    }
    auto new_ptr = &jungle.at(new_pos.y, new_pos.x);
    if ((new_ptr->value == invalid) || (new_ptr->value == wall)) {
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
    current_ptr = new_ptr;
    pos = new_pos;
    return true;
  };
  for (auto direction : directions) {
    if (direction >= 0) {
      facing = static_cast<facing_t>(direction);
      continue;
    }
    int num_steps = -direction;
    auto diff = get_diff();
    current_ptr = &jungle.at(pos.y, pos.x);
    for (int i = 0; i < num_steps; ++i) {
      if (!try_move(diff)) {
        break;
      }
    }
  }
  return {pos, facing};
}

template <bool part2>
int solve_case(const std::string& filename) {

  row_t current_ptr_row;
  std::string new_line;
  std::vector<row_t> raw_map;

  min_max_helper grid_size;

  const auto parse_map_raw = [&](std::string_view line, int row) {
    min_max_helper horizontal_limits;
    int column = -1;
    new_line.clear();
    std::ranges::transform(line, std::back_inserter(new_line),
                           [&](const char value) {
                             ++column;
                             switch (value) {
                               case empty:
                                 [[fallthrough]];
                               case wall:
                                 grid_size.update({column, row});
                                 horizontal_limits.update({column, 0});
                                 return value;
                               default:
                                 return static_cast<char>(invalid);
                             }
                           });
    // Needs to point past the end
    // We'll update vertical limits later
    horizontal_limits.max_value.x += 1;

    current_ptr_row.clear();
    std::ranges::transform(new_line, std::back_inserter(current_ptr_row),
                           [&](const char value) {
                             return cell_t{value, horizontal_limits};
                           });
    raw_map.push_back(std::move(current_ptr_row));
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
  jungle_t jungle;

  for (auto& raw_row : raw_map) {
    for (int column = raw_row.size(); column < row_length; ++column) {
      raw_row.push_back(cell_t{invalid, grid_size});
    }
    jungle.add_row(raw_row);
  }

  // Adjust top-to-bottom-limits
  for (int column = 0; column < jungle.row_length(); ++column) {
    min_max_helper vertical_limits;
    for (int row = 0; row < jungle.num_rows(); ++row) {
      const auto& value = jungle.at(row, column);
      if ((value == empty) || (value == wall)) {
        vertical_limits.update({0, row});
      }
    }
    vertical_limits.max_value.y += 1;
    for (int row = 0; row < jungle.num_rows(); ++row) {
      auto& value = jungle.at(row, column);
      if ((value == empty) || (value == wall)) {
        value.limits.min_value.y = vertical_limits.min_value.y;
        value.limits.max_value.y = vertical_limits.max_value.y;
      }
    }
  }

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
  AOC_EXPECT_RESULT(6032, (solve_case<false>("day22.example")));
  AOC_EXPECT_RESULT(97356, (solve_case<false>("day22.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(301, (solve_case<true>("day22.example")));
  // AOC_EXPECT_RESULT(3229579395609, (solve_case<true>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
