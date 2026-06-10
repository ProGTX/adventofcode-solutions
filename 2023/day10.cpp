// https://adventofcode.com/2023/day/10

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <span>
#include <string>

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

using field_t = aoc::char_grid<>;

struct Input {
  field_t field;
  int start_index;
};

fn parse(String const& filename) -> Input {
  auto field = aoc::read_char_grid(filename);
  let start_index = static_cast<i32>(
      stdr::distance(field.begin(), stdr::find(field, tile_t::start)));
  return Input{std::move(field), start_index};
}

fn get_pipe_loop(field_t const& field, i32 start_index) -> Vec<i32> {
  auto loop_indexes = Vec<i32>{};
  loop_indexes.push_back(start_index);
  auto start_neighbor_diffs = aoc::static_vector<point, 2>{};

  auto pos_2d = field.position(start_index);
  let south_diff = aoc::get_diff(aoc::south);
  let north_diff = aoc::get_diff(aoc::north);
  let west_diff = aoc::get_diff(aoc::west);
  let east_diff = aoc::get_diff(aoc::east);

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
    if (!stdr::contains(valid_values, value)) {
      return;
    }
    start_neighbor_diffs.push_back(diff);
  };

  {
    using namespace tile_t;

    add_starting_neighbor(pos_2d, south_diff, south_allowed);
    add_starting_neighbor(pos_2d, north_diff, north_allowed);
    add_starting_neighbor(pos_2d, west_diff, west_allowed);
    add_starting_neighbor(pos_2d, east_diff, east_allowed);

    auto start_neighbor_indexes =
        stdv::transform(start_neighbor_diffs,
                        [&](point diff) {
                          auto pos = pos_2d + diff;
                          return field.linear_index(pos.y, pos.x);
                        }) |
        aoc::collect_static_vec<i32, 2>();
    loop_indexes.push_back(start_neighbor_indexes[0]);
  }

  auto get_neighbors_no_check =
      [&](i32 current_index) -> aoc::static_vector<point, 2> {
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
        AOC_UNREACHABLE("Invalid pipe tile");
        return {point{}, point{}};
    }
  };

  auto current_index = loop_indexes.back();
  while (true) {
    let neighbors = get_neighbors_no_check(current_index);

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

fn get_num_steps(field_t const& field, i32 start_index) -> i32 {
  return static_cast<i32>(get_pipe_loop(field, start_index).size()) / 2;
}

fn test_field() {
  return field_t{"....."
                 ".S-7."
                 ".|.|."
                 ".L-J."
                 ".....",
                 5, 5};
};

static_assert(4 == get_num_steps(test_field(), 6));
static_assert(stdr::equal(get_pipe_loop(test_field(), 6),
                          std::array{6, 11, 16, 17, 18, 13, 8, 7}));

fn solve_case1(Input const& input) -> i32 {
  return get_num_steps(input.field, input.start_index);
}

// https://www.reddit.com/r/adventofcode/comments/18fgddy/2023_day_10_part_2_using_a_rendering_algorithm_to/
// https://en.wikipedia.org/wiki/Point_in_polygon
fn num_inside(field_t const& field, i32 start_index) -> i32 {
  let pipe_loop = get_pipe_loop(field, start_index);
  auto corners = Vec<point>{};
  for (let index : pipe_loop) {
    let value = field.at_index(index);
    if ((value == tile_t::vertical_pipe) ||
        (value == tile_t::horizontal_pipe)) {
      continue;
    }
    corners.push_back(field.position(index));
  }
  let area = aoc::calculate_area(std::span{corners});
  return area - static_cast<i32>(pipe_loop.size()) / 2 + 1;
}

static_assert(1 == num_inside(test_field(), 6));

fn solve_case2(Input const& input) -> i32 {
  return num_inside(input.field, input.start_index);
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  AOC_EXPECT_RESULT(4, solve_case1(example));
  let example2 = parse("day10.example2");
  AOC_EXPECT_RESULT(8, solve_case1(example2));
  let input = parse("day10.input");
  AOC_EXPECT_RESULT(6820, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1, solve_case2(example));
  AOC_EXPECT_RESULT(1, solve_case2(example2));
  let example3 = parse("day10.example3");
  AOC_EXPECT_RESULT(4, solve_case2(example3));
  AOC_EXPECT_RESULT(337, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
