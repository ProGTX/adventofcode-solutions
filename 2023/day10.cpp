// https://adventofcode.com/2023/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
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
inline constexpr char north_east_L = 'L';
inline constexpr char north_west_J = 'J';
inline constexpr char south_east_F = 'F';
inline constexpr char south_west_7 = '7';
inline constexpr char ground = '.';
inline constexpr char start = 'S';

inline constexpr auto north_allowed =
    std::array{vertical_pipe, south_east_F, south_west_7};
inline constexpr auto south_allowed =
    std::array{vertical_pipe, north_east_L, north_west_J};
inline constexpr auto west_allowed =
    std::array{horizontal_pipe, north_east_L, south_east_F};
inline constexpr auto east_allowed =
    std::array{horizontal_pipe, north_west_J, south_west_7};

} // namespace tile_t

using field_t = aoc::grid<char, std::string, std::vector<char>>;

constexpr std::vector<int> get_pipe_loop(const field_t& field,
                                         const int start_index) {
  std::vector<int> loop_indexes;
  loop_indexes.push_back(start_index);
  aoc::static_vector<point, 2> start_neighbor_diffs;

  // Separate logic for finding the neighbors of the start position
  auto add_starting_neighbor = [&](point current_pos, point diff,
                                   std::array<char, 3> valid_values) {
    if (start_neighbor_diffs.size() > 1) {
      return;
    }
    auto pos = current_pos + diff;
    if (!field.in_bounds(pos.y, pos.x)) {
      return;
    }
    auto value = field.at(pos.y, pos.x);
    AOC_ASSERT(
        value != tile_t::start,
        "Something went wrong, cannot use start as neighbor at this point");
    if (!aoc::ranges::contains(valid_values, value)) {
      return;
    }
    start_neighbor_diffs.push_back(diff);
  };

  auto pos_2d = field.position(start_index);
  const auto south_diff = aoc::get_diff(aoc::south);
  const auto north_diff = aoc::get_diff(aoc::north);
  const auto west_diff = aoc::get_diff(aoc::west);
  const auto east_diff = aoc::get_diff(aoc::east);
  {
    using namespace tile_t;

    add_starting_neighbor(pos_2d, south_diff, south_allowed);
    add_starting_neighbor(pos_2d, north_diff, north_allowed);
    add_starting_neighbor(pos_2d, west_diff, west_allowed);
    add_starting_neighbor(pos_2d, east_diff, east_allowed);

    auto start_neighbor_indexes =
        std::views::transform(start_neighbor_diffs,
                              [&](point diff) {
                                auto pos = pos_2d + diff;
                                return field.linear_index(pos.y, pos.x);
                              }) |
        aoc::ranges::to<aoc::static_vector<int, 2>>();
    loop_indexes.push_back(start_neighbor_indexes[0]);
  }

  auto get_neighbors_no_check =
      [&](int current_index) -> aoc::static_vector<point, 2> {
    pos_2d = field.position(current_index);
    auto current_value = field.at_index(current_index);
    // No need to check edges, assume each pipe always has two neighbors
    using namespace tile_t;
    switch (current_value) {
      case vertical_pipe:
        return {pos_2d + north_diff, pos_2d + south_diff};
      case horizontal_pipe:
        return {pos_2d + east_diff, pos_2d + west_diff};
      case north_east_L:
        return {pos_2d + north_diff, pos_2d + east_diff};
      case north_west_J:
        return {pos_2d + north_diff, pos_2d + west_diff};
      case south_east_F:
        return {pos_2d + south_diff, pos_2d + east_diff};
      case south_west_7:
        return {pos_2d + south_diff, pos_2d + west_diff};
      default:
        AOC_ASSERT(false, "Invalid pipe tile");
        return {point{}, point{}};
    }
  };

  int current_index = loop_indexes.back();
  while (true) {
    const auto neighbors = get_neighbors_no_check(current_index);

    auto neighbor_index = field.linear_index(neighbors[0].y, neighbors[0].x);
    if (!field.in_bounds(neighbors[0].y, neighbors[0].x) ||
        (loop_indexes[loop_indexes.size() - 2] == neighbor_index)) {
      neighbor_index = field.linear_index(neighbors[1].y, neighbors[1].x);
    }
    if (neighbor_index == start_index) {
      break;
    }
    loop_indexes.push_back(neighbor_index);
    current_index = neighbor_index;
  }
  return loop_indexes;
}

constexpr int get_num_steps(const field_t& field, const int start_index) {
  return get_pipe_loop(field, start_index).size() / 2;
}

constexpr auto test_field() {
  return field_t{std::vector<char>{
                     '.', '.', '.', '.', '.', //
                     '.', 'S', '-', '7', '.', //
                     '.', '|', '.', '|', '.', //
                     '.', 'L', '-', 'J', '.', //
                     '.', '.', '.', '.', '.'  //
                 },
                 5, 5};
};

static_assert(4 == get_num_steps(test_field(), 6));
static_assert(std::ranges::equal(get_pipe_loop(test_field(), 6),
                                 std::array{6, 11, 16, 17, 18, 13, 8, 7}));

// https://www.reddit.com/r/adventofcode/comments/18fgddy/2023_day_10_part_2_using_a_rendering_algorithm_to/
// https://en.wikipedia.org/wiki/Point_in_polygon
constexpr int num_inside(const field_t& field, const int start_index) {
  const auto pipe_loop = get_pipe_loop(field, start_index);
  std::vector<point> corners;
  for (auto index : pipe_loop) {
    const auto value = field.at_index(index);
    if ((value == tile_t::vertical_pipe) ||
        (value == tile_t::horizontal_pipe)) {
      continue;
    }
    corners.push_back(field.position(index));
  }
  int area = aoc::calculate_area(std::span{corners});
  return area - (pipe_loop.size() / 2) + 1;
}

static_assert(1 == num_inside(test_field(), 6));

template <bool calc_area>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;
  field_t field;
  int start_index = 0;

  for (std::string& line : aoc::views::read_lines(filename)) {
    if (auto start_pos = line.find(tile_t::start);
        start_pos != std::string::npos) {
      start_index = field.size() + start_pos;
    }
    field.add_row(std::move(line));
  }

  int sum = 0;
  if constexpr (!calc_area) {
    sum = get_num_steps(field, start_index);
  } else {
    sum = num_inside(field, start_index);
  }
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4, (solve_case<false>("day10.example")));
  AOC_EXPECT_RESULT(8, (solve_case<false>("day10.example2")));
  AOC_EXPECT_RESULT(6820, (solve_case<false>("day10.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1, (solve_case<true>("day10.example")));
  AOC_EXPECT_RESULT(1, (solve_case<true>("day10.example2")));
  AOC_EXPECT_RESULT(4, (solve_case<true>("day10.example3")));
  AOC_EXPECT_RESULT(337, (solve_case<true>("day10.input")));
  AOC_RETURN_CHECK_RESULT();
}
