// https://adventofcode.com/2024/day/6

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <type_traits>
#include <unordered_map>

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

// The guard only ever faces one of the 4 cardinal directions,
// so its direction can be packed into a 2-bit index.
constexpr i32 direction_bit(point d) {
  if (d == point{0, -1}) {
    return 0;
  } else if (d == point{1, 0}) {
    return 1;
  } else if (d == point{0, 1}) {
    return 2;
  } else {
    return 3; // {-1, 0}
  }
}

template <bool interfere, bool copy_map = false>
fn follow_guard(std::conditional_t<
                    copy_map, lab_map_t,
                    std::conditional_t<interfere, lab_map_t const&, lab_map_t&>>
                    lab_map,
                point const start_pos)
    -> std::conditional_t<interfere, bool, usize> {
  let start_direction = aoc::get_diff(aoc::north);
  auto pos = start_pos;
  auto direction = start_direction;
  // Maps each visited position to a bitmask of the directions
  // it has been visited from (see direction_bit)
  auto visited =
      std::conditional_t<interfere, std::unordered_map<point, u8>, usize>{};
  loop {
    if constexpr (!interfere) {
      lab_map.at(pos.y, pos.x) = visited_space;
    } else {
      auto& mask = visited[pos];
      let bit = static_cast<u8>(1 << direction_bit(direction));
      if (mask & bit) {
        // Detected a loop
        return true;
      }
      mask |= bit;
    }
    // Try to turn right, but might need to do it multiple times, at most 3
    // Stop rotating as soon as a non-obstacle cell is found, which may be
    // the edge -- in which case the walk ends below
    let rotate_right = [](point d) {
      d *= (d.x == 0) ? -1 : 1;
      std::swap(d.x, d.y);
      return d;
    };
    let candidate_directions =
        std::array{direction, rotate_right(direction),
                   rotate_right(rotate_right(direction))};
    let chosen_it = stdr::find_if(candidate_directions, [&](point d) {
      return lab_map.at((pos + d).y, (pos + d).x) != obstacle;
    });
    direction = (chosen_it != candidate_directions.end())
                    ? *chosen_it
                    : candidate_directions.back();
    let new_pos = pos + direction;
    if (lab_map.at(new_pos.y, new_pos.x) == edge) {
      break;
    }
    pos = new_pos;
  }
  if constexpr (!interfere) {
    return stdr::count(lab_map, visited_space);
  } else {
    return false;
  }
}

fn interfere_with_guard(lab_map_t& lab_map, point const start_pos) -> usize {
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
    let looped = follow_guard<true>(lab_map, start_pos);
    lab_map.at(pos.y, pos.x) = empty_space;
    stdr::replace(lab_map, visited_space, empty_space);
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

static_assert(41 == follow_guard<false, true>(test_map(), {5, 7}));

template <bool interfere>
fn solve_case(Input const& input) -> usize {
  // follow_guard/interfere_with_guard mutate the map,
  // so each part needs to start from its own copy
  auto lab_map = input.lab_map;
  if constexpr (!interfere) {
    return follow_guard<false>(lab_map, input.starting_guard_pos);
  } else {
    return interfere_with_guard(lab_map, input.starting_guard_pos);
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day06.example");
  AOC_EXPECT_RESULT(41, solve_case<false>(example));
  let input = parse("day06.input");
  AOC_EXPECT_RESULT(5030, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_case<true>(example));
  AOC_EXPECT_RESULT(1928, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
