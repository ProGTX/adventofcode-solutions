// https://adventofcode.com/2024/day/6

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <type_traits>

constexpr let empty_space = '.';
constexpr let obstacle = '#';
constexpr let starting_guard = '^';
constexpr let visited_space = 'X';

// We'll add edges to the map to remove the need for bounds checking
constexpr let edge = '?';

using lab_map_t = aoc::char_grid<>;

struct Input {
  lab_map_t lab_map;
  point starting_guard_pos;
};

auto parse(String const& filename) -> Input {
  auto [lab_map, config] = aoc::read_char_grid(
      filename,
      {.padding = edge, .start_char = starting_guard, .end_char = {}});
  return {std::move(lab_map), *config.start_pos};
}

// The guard only ever faces one of the 4 basic directions
// Rather than tracking direction as a point and rotating it,
// we track it as a facing_t index
// Turning right becomes a plain `(direction + 1) % aoc::NUM_FACING`
// instead of a rotation on a point,
// since the directions are already ordered clockwise

template <bool interfere, bool copy_map = false>
fn follow_guard(
    std::conditional_t<
        copy_map, lab_map_t,
        std::conditional_t<interfere, lab_map_t const&, lab_map_t&>>
        lab_map,
    point const start_pos,
    // Flat, per-cell bitmask of the directions each position has
    // been visited from (see the direction encoding above)
    // Only meaningful when interfere is true -
    // passed in and reused across calls by the caller
    std::conditional_t<interfere, Vec<u8>&, usize> visited_directions)
    -> std::conditional_t<interfere, bool, usize> {
  // Everything below operates on linear indices into the grid
  // rather than (row, column) points:
  // moving is just `index + deltas[direction]`,
  // and `cells` gives raw, unchecked read access to the underlying buffer
  let columns = static_cast<isize>(lab_map.num_columns());
  // Indexed by direction: east, south, west, north
  let deltas = std::array{isize{1}, columns, isize{-1}, -columns};
  let cells = lab_map.data().data();

  auto index = static_cast<isize>(lab_map.linear_index(start_pos));
  auto direction = static_cast<isize>(aoc::north);
  loop {
    if constexpr (!interfere) {
      lab_map.at_index(index) = visited_space;
    } else {
      auto& mask = visited_directions[index];
      let bit = static_cast<u8>(1 << direction);
      if (mask & bit) {
        // Detected a loop
        return true;
      }
      mask |= bit;
    }
    // Try to turn right, but might need to do it multiple times, at most 3
    // Stop rotating as soon as a non-obstacle cell is found, which may be
    // the edge -- in which case the walk ends below
    let candidate_directions =
        std::array{direction, (direction + 1) % aoc::NUM_FACING,
                   (direction + 2) % aoc::NUM_FACING};
    let chosen_it = stdr::find_if(candidate_directions, [&](isize d) {
      return cells[index + deltas[d]] != obstacle;
    });
    direction = (chosen_it != candidate_directions.end())
                    ? *chosen_it
                    : candidate_directions.back();
    let new_index = index + deltas[direction];
    if (cells[new_index] == edge) {
      break;
    }
    index = new_index;
  }
  if constexpr (!interfere) {
    return stdr::count(lab_map, visited_space);
  } else {
    return false;
  }
}

fn interfere_with_guard(lab_map_t lab_map, point const start_pos) -> usize {
  // Reused across every candidate obstacle position
  auto visited_directions = Vec<u8>(lab_map.size(), u8{0});

  // Checks whether placing an obstacle at pos would catch the guard in a loop
  // Ignores edges and the guard itself
  let is_looping_obstacle = [&](point pos) -> bool {
    if (pos == start_pos) [[unlikely]] {
      return false;
    }
    if (lab_map.at(pos.y, pos.x) != empty_space) [[unlikely]] {
      return false;
    }
    lab_map.at(pos.y, pos.x) = obstacle;
    stdr::fill(visited_directions, u8{0});
    let looped = follow_guard<true>(lab_map, start_pos, visited_directions);
    lab_map.at(pos.y, pos.x) = empty_space;
    return looped;
  };

  // Go through each point on the map and check if placing an obstacle there
  // would catch the guard in a loop
  return aoc::ranges::accumulate(
      Range{1uz, lab_map.num_rows() - 1} | stdv::transform([&](usize row) {
        return stdr::count_if(
            Range{1uz, lab_map.num_columns() - 1}, [&](usize col) {
              return is_looping_obstacle(
                  point{static_cast<i32>(col), static_cast<i32>(row)});
            });
      }),
      usize{});
}

fn test_map() -> lab_map_t {
  return //
      {String{"????????????"
              "?....#.....?"
              "?.........#?"
              "?..........?"
              "?..#.......?"
              "?.......#..?"
              "?..........?"
              "?.#..^.....?"
              "?........#.?"
              "?#.........?"
              "?......#...?"
              "????????????"},
       12, 12};
}

static_assert(41 == follow_guard<false, true>(test_map(), {5, 7}, usize{}));

fn solve_part1(Input const& input) -> usize {
  // follow_guard mutates the map, so start from a copy
  auto lab_map = input.lab_map;
  return follow_guard<false>(lab_map, input.starting_guard_pos, usize{});
}

fn solve_part2(Input const& input) -> usize {
  return interfere_with_guard(input.lab_map, input.starting_guard_pos);
}

int main() {
  std::println("Part 1");
  let example = parse("day06.example");
  AOC_EXPECT_RESULT(41, solve_part1(example));
  let input = parse("day06.input");
  AOC_EXPECT_RESULT(5030, solve_part1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_part2(example));
  AOC_EXPECT_RESULT(1928, solve_part2(input));

  AOC_RETURN_CHECK_RESULT();
}
