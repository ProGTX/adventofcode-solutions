// https://adventofcode.com/2022/day/22

#include "../common/common.h"

#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using parsed_map_t = aoc::char_grid<>;
constexpr const char empty_char = ' ';
constexpr const char tile_char = '.';
constexpr const char wall_char = '#';

// Encode each position
// Edges of the box/cube specify the index of the connected tile
// The index is encoded as a linear index into the grid,
// which is at most 152x202 in size
// However each empty space on the edge needs to distinguish direction as well,
// so it needs to store two indexes
// The inside tiles then need to be larger than any index
using jungle_t = aoc::grid<int>;
constexpr const int empty_space = 1000 * 1000;
constexpr const int empty_tile = 1000 * 1000 + 1;
constexpr const int wall_tile = 1000 * 1000 + 2;

constexpr const char clockwise_char = 'R';
constexpr const char counterclockwise_char = 'L';
constexpr const int turn_clockwise = -1;
constexpr const int turn_counterclockwise = -2;

constexpr parsed_map_t parse_map(std::span<std::string> raw_map,
                                 point map_size) {
  // Add empty space all around the map to remove the need for bounds checking
  map_size += {2, 2};
  parsed_map_t parsed_map{};
  for (std::string& line : raw_map) {
    if (parsed_map.empty()) {
      parsed_map.add_row(std::views::repeat(empty_char, map_size.x));
    }
    auto empty_suffix =
        std::views::repeat(empty_char, map_size.x - line.size() - 1) |
        aoc::ranges::to<std::string>();
    parsed_map.add_row(empty_char + std::move(line) + std::move(empty_suffix));
  }
  parsed_map.add_row(std::views::repeat(empty_char, map_size.x));
  return parsed_map;
}

template <bool is_cube>
constexpr jungle_t get_jungle(const parsed_map_t& parsed_map) {
  const auto num_rows = parsed_map.num_rows();
  const auto num_columns = parsed_map.num_columns();
  jungle_t jungle{empty_space, num_rows, num_columns};
  const auto size = jungle.size();

  const auto get_pos = [](const int primary_index, const int nested_index,
                          const bool is_row) {
    point pos{primary_index, nested_index};
    if (is_row) {
      std::swap(pos.x, pos.y);
    }
    return pos;
  };

  using line_t = std::string;
  auto display_map = parsed_map;

  const auto set_edge = [&](const point edge_pos, const int primary_index,
                            const int nested_index, const bool is_row) {
    const auto jump_pos = get_pos(primary_index, nested_index, is_row);
    auto linear_index = jungle.linear_index(jump_pos.y, jump_pos.x);
    if (is_row) {
      linear_index *= size;
    }
    const auto edge_value = jungle.at(edge_pos.y, edge_pos.x);
    if (edge_value != empty_space) {
      // Edge has already been modified, so we need to add to it
      // This happens in corners
      linear_index += edge_value;
    }
    jungle.modify(linear_index, edge_pos.y, edge_pos.x);
  };

  const auto add_line = [&](const line_t& line, const int primary_index,
                            const bool is_row) {
    // Boundaries hold indexes to non-empty tiles
    std::pair<int, int> empty_bounds{-1, -1};
    auto it = std::ranges::begin(line);
    for (int nested_index = 0; nested_index < line.size();
         ++nested_index, ++it) {
      const auto pos = get_pos(primary_index, nested_index, is_row);
      const char current = *it;
      if (current != empty_char) {
        if (empty_bounds.first == -1) {
          // First non-empty tile is a boundary
          empty_bounds.first = nested_index;
          // This will be processed when we get to the end of non-empty tiles
        }
      } else if (empty_bounds.first > 0) {
        if (empty_bounds.second == -1) {
          // First empty tile after non-empty tiles is a boundary
          empty_bounds.second = nested_index - 1;

          // Process lower edge (we're currently here)
          // Inner tile jumps to the upper edge
          set_edge(pos, primary_index, empty_bounds.first, is_row);

          // Process upper edge
          // Upper edge jump to the lower edge
          // Minus 1 because we're modifying empty space before inner tiles
          const auto upper_edge_pos =
              get_pos(primary_index, empty_bounds.first - 1, is_row);
          set_edge(upper_edge_pos, primary_index, empty_bounds.second, is_row);
        } else {
          // Just empty space
        }
      } else {
        // Just empty space
      }
      if (current == tile_char) {
        jungle.modify(empty_tile, pos.y, pos.x);
      } else if (current == wall_char) {
        jungle.modify(wall_tile, pos.y, pos.x);
      }
    }
  };

  // Iterate over rows and columns
  // Skip the empty edges of the map
  for (int index : std::views::iota(1, static_cast<int>(num_rows - 1))) {
    add_line(parsed_map.row_view<line_t>(index), index, true);
  }
  for (int index : std::views::iota(1, static_cast<int>(num_columns - 1))) {
    add_line(parsed_map.column_view<line_t>(index), index, false);
  }

  return jungle;
}

using arrow_t = aoc::arrow_type<int>;

constexpr arrow_t walk_through(const jungle_t& jungle,
                               std::span<const int> steps) {
  const auto size = jungle.size();
  auto current = [&]() {
    // Starting position is the left-most empty tile on the top (non-empty) row
    auto it = std::ranges::find(jungle.row_view(1), empty_tile);
    return arrow_t{point(std::ranges::distance(jungle.begin_row(1), it), 1),
                   aoc::east};
  }();
  for (int step : steps) {
    if (step == turn_clockwise) {
      current.direction = aoc::clockwise_basic(current.direction);
    } else if (step == turn_counterclockwise) {
      current.direction = aoc::anticlockwise_basic(current.direction);
    }
    for (int i = 0; i < step; ++i) {
      auto next_pos = current.position + aoc::get_diff(current.direction);
      auto next_value = jungle.at(next_pos.y, next_pos.x);
      if (next_value == wall_tile) {
        break;
      } else if (next_value == empty_tile) {
        current.position = next_pos;
      } else {
        AOC_ASSERT(next_value != empty_space, "Should never reach empty space");
        // next_value acts as a linear index in this case
        auto index = next_value;
        const auto dir = current.direction;
        if ((dir == aoc::east) || (dir == aoc::west)) {
          index = index / size;
        } else {
          index = index % size;
        }

        next_pos = jungle.position(index);
        next_value = jungle.at(next_pos.y, next_pos.x);
        if (next_value == wall_tile) {
          break;
        } else {
          AOC_ASSERT(next_value == empty_tile,
                     "Can only jump into inner tiles");
        }
      }
      current.position = next_pos;
    }
  }
  return current;
}

template <bool is_cube, int cube_side>
int solve_case(const std::string& filename) {
  std::vector<int> steps;

  // The map doesn't have equal size rows, so we need to parse it in two steps
  std::vector<std::string> raw_map;
  aoc::min_max_helper min_max{};

  bool parsing_steps = false;
  for (int height = 1; std::string line : aoc::views::read_lines(
                           filename, aoc::keep_empty{}, aoc::keep_spaces{})) {
    if (line.empty()) {
      parsing_steps = true;
      continue;
    }
    if (!parsing_steps) {
      min_max.update(point(line.size(), height));
      raw_map.push_back(std::move(line));
      ++height;
    } else {
      int start = 0;
      int size = 0;
      const auto get_step_size = [&]() {
        steps.push_back(aoc::to_number<int>(line.substr(start, size)));
      };
      for (int i = 0; i < line.size(); ++i) {
        const char current = line[i];
        if (current == clockwise_char) {
          get_step_size();
          steps.push_back(turn_clockwise);
          start = i + 1;
          size = 0;
        } else if (current == counterclockwise_char) {
          get_step_size();
          steps.push_back(turn_counterclockwise);
          start = i + 1;
          size = 0;
        } else {
          ++size;
        }
      }
      get_step_size();
    }
  }

  const auto [pos, facing] = walk_through(
      get_jungle<is_cube>(parse_map(raw_map, min_max.max_value)), steps);

  auto password = 1000 * pos.y + 4 * pos.x + facing;
  std::cout << filename << " -> " << password << std::endl;
  return password;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6032, (solve_case<false, 4>("day22.example")));
  AOC_EXPECT_RESULT(97356, (solve_case<false, 50>("day22.input")));
  std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(5031, (solve_case<true, 4>("day22.example")));
  // AOC_EXPECT_RESULT(3229579395609, (solve_case<true, 50>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
