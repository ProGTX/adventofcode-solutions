// https://adventofcode.com/2023/day/10

#include "../common/common.h"
#include "../common/flat.h"
#include "../common/grid.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

namespace tile_t {

inline constexpr char vertical_pipe = '|';
inline constexpr char horizontal_pipe = '-';
inline constexpr char north_east_pipe = 'L';
inline constexpr char north_west_pipe = 'J';
inline constexpr char south_east_pipe = 'F';
inline constexpr char south_west_pipe = '7';
inline constexpr char ground = '.';
inline constexpr char start = 'S';

} // namespace tile_t

using field_t = grid<char, std::string, std::vector<char>>;

struct neighbor_t {
  int index;
  int distance;

  constexpr auto operator<=>(const neighbor_t& other) const {
    return distance <=> other.distance;
  }

  constexpr bool operator==(const neighbor_t& other) const = default;
};

inline constexpr auto unvisited_char = static_cast<char>(0);
inline constexpr auto visited_char = static_cast<char>(1);

constexpr int get_num_steps(const field_t& field, const int start_index) {
  sorted_flat_set<neighbor_t> unvisited_indexes;

  // Separate logic for adding neighbors of the start position
  auto add_starting_neighbor = [&](point pos, std::string_view valid_values) {
    if (!field.in_bounds(pos.y, pos.x)) {
      return;
    }
    auto value = field.at(pos.y, pos.x);
    AOC_ASSERT(
        value != tile_t::start,
        "Something went wrong, cannot use start as neighbor at this point");
    if (ranges::contains(valid_values, value)) {
      unvisited_indexes.emplace(
          static_cast<int>(field.linear_index(pos.y, pos.x)), 1);
    }
  };
  auto pos_2d = field.position(start_index);
  {
    using namespace tile_t;
    add_starting_neighbor(
        pos_2d + point{0, 1}, // south
        construct_string<std::string_view>(
            std::array{vertical_pipe, north_east_pipe, north_west_pipe}));
    add_starting_neighbor(
        pos_2d + point{0, -1}, // north
        construct_string<std::string_view>(
            std::array{vertical_pipe, south_east_pipe, south_west_pipe}));
    add_starting_neighbor(
        pos_2d + point{1, 0}, // west
        construct_string<std::string_view>(
            std::array{horizontal_pipe, north_west_pipe, south_west_pipe}));
    add_starting_neighbor(
        pos_2d + point{-1, 0}, // east
        construct_string<std::string_view>(
            std::array{horizontal_pipe, north_east_pipe, south_east_pipe}));
  }

  auto get_neighbors_no_check =
      [&](neighbor_t current) -> static_vector<point, 2> {
    pos_2d = field.position(current.index);
    auto current_value = field.at_index(current.index);
    // No need to check edges, assume each pipe always has two neighbors
    using namespace tile_t;
    switch (current_value) {
      case vertical_pipe:
        return {pos_2d + point{0, -1}, pos_2d + point{0, 1}};
      case horizontal_pipe:
        return {pos_2d + point{1, 0}, pos_2d + point{-1, 0}};
      case north_east_pipe:
        return {pos_2d + point{0, -1}, pos_2d + point{1, 0}};
      case north_west_pipe:
        return {pos_2d + point{0, -1}, pos_2d + point{-1, 0}};
      case south_east_pipe:
        return {pos_2d + point{0, 1}, pos_2d + point{1, 0}};
      case south_west_pipe:
        return {pos_2d + point{0, 1}, pos_2d + point{-1, 0}};
      default:
        AOC_ASSERT(false, "Invalid pipe tile");
        return {point{}, point{}};
    }
  };

  int max_distance = 0;
  std::vector<char> visited_index_map(field.size(), unvisited_char);
  auto add_neighbors_to_unvisited = [&](neighbor_t current) {
    const auto neighbors = get_neighbors_no_check(current);
    for (const point& neigh : neighbors) {
      auto index = field.linear_index(neigh.y, neigh.x);
      if (visited_index_map[index] == visited_char) {
        continue;
      }
      auto new_distance = current.distance + 1;
      unvisited_indexes.emplace(static_cast<int>(index), new_distance);
      if (new_distance > max_distance) {
        max_distance = new_distance;
      }
    }
  };

  visited_index_map[start_index] = visited_char;
  while (unvisited_indexes.size()) {
    neighbor_t current = *std::begin(unvisited_indexes);
    visited_index_map[current.index] = visited_char;
    add_neighbors_to_unvisited(current);
    unvisited_indexes.erase(current);
  }
  return max_distance;
}

static_assert(4 == get_num_steps(field_t{std::vector<char>{
                                             '.', '.', '.', '.', '.', //
                                             '.', 'S', '-', '7', '.', //
                                             '.', '|', '.', '|', '.', //
                                             '.', 'L', '-', 'J', '.', //
                                             '.', '.', '.', '.', '.'  //
                                         },
                                         5, 5},
                                 6));

template <bool>
int solve_case(const std::string& filename) {
  field_t field;
  int start_index = 0;

  auto read_values = [&](std::string&& line) {
    if (auto start_pos = line.find(tile_t::start);
        start_pos != std::string::npos) {
      start_index = field.size() + start_pos;
    }
    field.add_row(std::move(line));
  };
  readfile_op(filename, read_values);

  int sum = get_num_steps(field, start_index);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4, (solve_case<false>("day10.example")));
  AOC_EXPECT_RESULT(8, (solve_case<false>("day10.example2")));
  AOC_EXPECT_RESULT(6820, (solve_case<false>("day10.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(2, (solve_case<true>("day10.example")));
  // AOC_EXPECT_RESULT(977, (solve_case<true>("day10.input")));
  AOC_RETURN_CHECK_RESULT();
}
