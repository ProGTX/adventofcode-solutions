// https://adventofcode.com/2022/day/17

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <print>
#include <ranges>

namespace stdr = std::ranges;
namespace stdv = std::views;

using rock_index_t = i64;
using Input = Vec<int>;
using grid_point = aoc::point_type<rock_index_t>;
constexpr let rock_size = 5;
using rock_t = std::array<grid_point, rock_size>;

fn parse(String const& filename) -> Input {
  auto lines = aoc::views::read_lines(filename);
  return str{*stdr::begin(lines)} |
         stdv::transform([](char c) { return (c == '<') ? -1 : 1; }) |
         aoc::ranges::to<Input>();
}

fn move_rock(rock_t& rock, grid_point delta) {
  for (grid_point& p : rock) {
    p += delta;
  }
}

template <rock_index_t num_rocks>
fn solve_case(Input const& pattern) -> rock_index_t {
  constexpr let use_sparse_grid = num_rocks > 10000;

  constexpr int left_most_index = 0;
  constexpr int right_most_index = 4;
  constexpr int top_most_index = 1;
  constexpr int bottom_most_index = 3;

  constexpr let initial_rock_displacement = grid_point{2, 3};

  constexpr let rock_structures = std::invoke([&] {
    std::array<rock_t, 5> rocks;
    rocks[0] = {grid_point{0, 0}, grid_point{1, 0}, grid_point{2, 0},
                grid_point{3, 0}, grid_point{3, 0}};
    rocks[1] = {grid_point{0, 1}, grid_point{1, 2}, grid_point{1, 1},
                grid_point{1, 0}, grid_point{2, 1}};
    rocks[2] = {grid_point{0, 0}, grid_point{2, 2}, grid_point{2, 0},
                grid_point{1, 0}, grid_point{2, 1}};
    rocks[3] = {grid_point{0, 1}, grid_point{0, 3}, grid_point{0, 0},
                grid_point{0, 0}, grid_point{0, 2}};
    rocks[4] = {grid_point{0, 0}, grid_point{0, 1}, grid_point{0, 0},
                grid_point{1, 0}, grid_point{1, 1}};

    // Each rock appears so that its left edge is two units away
    // from the left wall and its bottom edge is three units above
    // the highest rock in the room (or the floor, if there isn't one).
    for (auto& rock : rocks) {
      move_rock(rock, initial_rock_displacement);
    }
    return rocks;
  });
  constexpr let num_rock_types = rock_structures.size();

  constexpr let empty_char = ' ';
  constexpr let wall_char = '|'; // floor and side walls
  constexpr let rock_char = '#'; // settled rocks
  constexpr let chamber_width = 7;

  // Just use rocks as the walls, no need to do bounds checking
  using chamber_row_t = std::array<char, chamber_width + 2>;

  auto chamber = std::invoke([] {
    if constexpr (use_sparse_grid) {
      return aoc::sparse_grid<char, char{}, grid_point, chamber_row_t>{};
    } else {
      return aoc::grid<char, chamber_row_t, std::vector<char>>{};
    }
  });

  // And use rocks for the floor
  {
    auto floor = chamber_row_t{};
    floor.fill(wall_char);
    chamber.add_row(floor);
  }

  constexpr let empty_new_row = std::invoke([&] {
    chamber_row_t new_row;
    new_row.fill(empty_char);
    new_row[0] = new_row[new_row.size() - 1] = wall_char;
    return new_row;
  });

  let try_move_rock = [&](rock_t& falling_rock, grid_point delta) -> bool {
    move_rock(falling_rock, delta);
    let intersects_with_rock =
        stdr::any_of(falling_rock, [&](const grid_point& rp) {
          return chamber.at(rp.y, rp.x) != empty_char;
        });
    if (intersects_with_rock) {
      // Invalid movement, move it back
      move_rock(falling_rock, -delta);
      return false;
    }
    return true;
  };

  auto current_height = rock_index_t{1};
  int pattern_index = 0;
  let rock_solver = [&](rock_index_t rock_index) {
    auto falling_rock =
        rock_structures[static_cast<usize>(rock_index % num_rock_types)];
    let current_rock_height =
        falling_rock[top_most_index].y - falling_rock[bottom_most_index].y + 1;

    // Do initial falling stage without a chamber
    for (let _ : Range{0, initial_rock_displacement.y}) {
      // Only do horizontal movement in  this loop
      auto sideways = pattern[pattern_index];
      auto leftmost = falling_rock[left_most_index].x + sideways;
      auto rightmost = falling_rock[right_most_index].x + sideways;
      if ((leftmost >= 0) && (rightmost < chamber_width)) {
        move_rock(falling_rock, grid_point{sideways, 0});
      }
      pattern_index = (pattern_index + 1) % pattern.size();
    }

    { // Place falling rock into the chamber
      let missing_rows = current_height +
                         current_rock_height -
                         static_cast<rock_index_t>(chamber.num_rows());
      for (rock_index_t r = 0; r < missing_rows; ++r) {
        chamber.add_row(empty_new_row);
      }
      // This move includes the vertical movement from the initial falling stage
      // It also includes the left chamber wall
      move_rock(falling_rock,
                grid_point{1, current_height - initial_rock_displacement.y});
    }

    // Move rock until it settles
    loop {
      auto sideways = pattern[pattern_index];
      try_move_rock(falling_rock, grid_point{sideways, 0});
      if (!try_move_rock(falling_rock, grid_point{0, -1})) {
        // Rock settled, engrave it into the chamber
        for (const grid_point& rp : falling_rock) {
          chamber.modify(rock_char, rp.y, rp.x);
        }
        current_height =
            std::max(current_height, falling_rock[top_most_index].y + 1);
        pattern_index = (pattern_index + 1) % pattern.size();
        break;
      }
      pattern_index = (pattern_index + 1) % pattern.size();
    }
  };

  // Cycle detection: the simulation is periodic once the same
  // (rock_type, pattern_index, surface_hash) state recurse.
  //
  // rock_type and pattern_index alone are not sufficient:
  // two states can share those while the chamber surface is shaped differently
  // (rocks settled differently in earlier iterations).
  // Including a hash of the top 30 rows makes false positives
  // practically impossible, since 30 rows is well beyond
  // how deep any rock can reach when settling.
  //
  // Once a repeated state is found:
  //   cycle_rocks: how many rocks fit in one period
  //   cycle_height: how much the tower grows per period
  // We skip as many full periods as possible and simulate the remainder

  // State key: what uniquely identifies the simulation state
  // rock_type, pattern_index, surface_hash
  using CycleKey = std::tuple<int, int, size_t>;
  // State value: where this state was last seen
  struct SeenState {
    rock_index_t rock_index;
    rock_index_t height;
  };

  // Polynomial rolling hash of the top 30 chamber rows.
  // Each character is folded in as: hash = hash * 131 + c.
  // 131 is a prime larger than the printable ASCII range,
  // which spreads bits well and keeps collisions rare.
  // A false positive (two different surfaces producing the same hash)
  // would yield a wrong answer, but with a 64-bit result
  // and only thousands of states visited before a cycle is found,
  // the collision probability is negligible.
  let surface_hash = [&] {
    size_t hash = 0;
    for (rock_index_t k = 0; (k < 30) && ((current_height - 1 - k) >= 0); ++k) {
      for (char c : chamber.get_row(current_height - 1 - k)) {
        hash = hash * 131 + static_cast<size_t>(c);
      }
    }
    return hash;
  };

  auto seen = std::map<CycleKey, SeenState>{};
  auto skipped_height = rock_index_t{};
  bool cycle_found = false;

  for (rock_index_t index = 0; index < num_rocks; ++index) {
    // Only compute the key and update the map until the cycle is detected,
    // afterwards just simulate the remaining leftover rocks.
    if (!cycle_found) {
      let rock_type = static_cast<int>(index % num_rock_types);
      let key = CycleKey{rock_type, pattern_index, surface_hash()};

      if (seen.contains(key)) {
        let[prev_i, prev_height] = seen[key];
        let cycle_rocks = index - prev_i;
        let cycle_height = (current_height - 1) - prev_height;
        let full_cycles = (num_rocks - index) / cycle_rocks;
        skipped_height = full_cycles * cycle_height;
        index += full_cycles * cycle_rocks;
        cycle_found = true;
        if (index >= num_rocks) {
          break;
        }
      } else {
        seen[key] = SeenState{
            .rock_index = index,
            .height = current_height - 1,
        };
      }
    }

    rock_solver(index);
  }

  // Physical height of simulated rocks
  // plus the height of all skipped periods
  return current_height - 1 + skipped_height;
}

int main() {
  std::println("Part 1");
  let example = parse("day17.example");
  AOC_EXPECT_RESULT(17, solve_case<10>(example));
  AOC_EXPECT_RESULT(3068, solve_case<2022>(example));
  let input = parse("day17.input");
  AOC_EXPECT_RESULT(3085, solve_case<2022>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1514285714288, solve_case<1000000000000>(example));
  AOC_EXPECT_RESULT(1535483870924, solve_case<1000000000000>(input));

  AOC_RETURN_CHECK_RESULT();
}
