// https://adventofcode.com/2022/day/17

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using rock_index_t = std::int64_t;
using grid_point = aoc::point_type<rock_index_t>;
using rock_t = std::array<grid_point, 5>;

constexpr void move_rock(rock_t& rock, grid_point move) {
  for (grid_point& p : rock) {
    p += move;
  }
}

template <rock_index_t num_rocks>
rock_index_t get_final_height(const std::vector<int>& pattern) {
  static constexpr bool use_sparse_grid = num_rocks > 10000;

  constexpr int left_most_index = 0;
  constexpr int right_most_index = 4;
  constexpr int top_most_index = 1;
  constexpr int bottom_most_index = 3;

  static constexpr auto initial_rock_displacement = grid_point{2, 3};

  constexpr std::array<rock_t, 5> rock_structures = std::invoke([] {
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

  static constexpr std::string_view rock_chars = "|=#$%*";
  int rock_char_index = 0;

  static constexpr char empty_char = ' ';
  // static constexpr char rock_char = '#';
  static constexpr int chamber_width = 7;

  // Just use rocks as the walls, no need to do bounds checking
  using chamber_row_t = std::array<char, chamber_width + 2>;

  auto chamber = std::invoke([]() {
    if constexpr (use_sparse_grid) {
      return aoc::sparse_grid<char, char{}, grid_point, chamber_row_t>{};
    } else {
      return aoc::grid<char, chamber_row_t, std::vector<char>>{};
    }
  });

  // And use rocks for the floor
  chamber_row_t floor;
  std::ranges::fill(floor, rock_chars[0]);
  chamber.add_row(floor);

  constexpr chamber_row_t empty_new_row = std::invoke([] {
    chamber_row_t new_row;
    std::ranges::fill(new_row, empty_char);
    new_row[0] = new_row[new_row.size() - 1] = rock_chars[0];
    return new_row;
  });

  const auto try_move_rock = [&](rock_t& falling_rock,
                                 grid_point move) -> bool {
    move_rock(falling_rock, move);
    bool intersects_with_rock =
        std::ranges::any_of(falling_rock, [&](const grid_point& rp) {
          const auto value = chamber.at(rp.y, rp.x);
          return std::ranges::any_of(
              rock_chars, [&](char rock_char) { return (value == rock_char); });
        });
    if (intersects_with_rock) {
      // Invalid movement, move it back
      move_rock(falling_rock, -move);
      return false;
    }
    return true;
  };

  rock_index_t current_height = 1;
  int pattern_index = 0;
  const auto rock_solver = [&](const int rock_index) {
    rock_char_index = (rock_char_index + 1) % rock_chars.size();

    rock_t falling_rock = rock_structures[rock_index % rock_structures.size()];
    const auto current_rock_height =
        falling_rock[top_most_index].y - falling_rock[bottom_most_index].y + 1;

    // Do initial falling stage without a chamber
    for (int y = 0; y < initial_rock_displacement.y; ++y) {
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
      auto missing_rows = current_height +
                          current_rock_height -
                          static_cast<rock_index_t>(chamber.num_rows());
      for (int r = 0; r < missing_rows; ++r) {
        chamber.add_row(empty_new_row);
      }
      // This move includes the vertical movement from the initial falling stage
      // It also includes the left chamber wall
      move_rock(falling_rock,
                grid_point{1, current_height - initial_rock_displacement.y});
    }

    // Move rock until it settles
    while (true) {
      auto sideways = pattern[pattern_index];
      try_move_rock(falling_rock, grid_point{sideways, 0});
      if (!try_move_rock(falling_rock, grid_point{0, -1})) {
        // Rock settled, engrave it into the chamber
        for (const grid_point& rp : falling_rock) {
          auto actual_rock_char_index =
              (rock_char_index % (rock_chars.size() - 1)) + 1;
          chamber.modify(rock_chars[actual_rock_char_index], rp.y, rp.x);
        }
        current_height =
            std::max(current_height, falling_rock[top_most_index].y + 1);
        pattern_index = (pattern_index + 1) % pattern.size();
        break;
      }
      pattern_index = (pattern_index + 1) % pattern.size();
    }
  };

  // Perform iterations

  const int cycle_size =
      std::min(static_cast<size_t>(num_rocks),
               std::lcm(rock_structures.size(), pattern.size()));
  int rock_index = 0;

  const auto [initial_num_rocks, initial_height,
              initial_top_row] = std::invoke([&]() {
    for (int r = 0; r < cycle_size; ++r) {
      rock_solver(rock_index);
      ++rock_index;
    }
    return std::tuple{static_cast<rock_index_t>(rock_index), current_height - 1,
                      chamber.get_row(current_height - 1)};
  });
  std::cout << aoc::print_range(initial_top_row) << std::endl;

  const auto is_same_as_initial = [&](const chamber_row_t& current_top_row) {
    for (int c = 1; c < initial_top_row.size() - 1; ++c) {
      if (current_top_row[c] == initial_top_row[c]) {
        continue;
      }
      if ((current_top_row[c] != empty_char) &&
          (initial_top_row[c] != empty_char)) {
        continue;
      }
      return false;
    }
    return true;
  };

  const auto [num_cycle_reps, rocks_extended_cycle,
              height_extended_cycle] = std::invoke([&]() {
    int num_cycle_reps = 0;
    while (rock_index < num_rocks) {
      ++num_cycle_reps;
      for (int r = 0; r < cycle_size; ++r) {
        rock_solver(rock_index);
        ++rock_index;
      }
      auto current_top_row = chamber.get_row(current_height - 1);
      std::cout << aoc::print_range(current_top_row) << std::endl;
      if (is_same_as_initial(current_top_row)) {
        break;
      }
    }
    return std::tuple{num_cycle_reps,
                      static_cast<rock_index_t>(rock_index - initial_num_rocks),
                      current_height - 1 - initial_height};
  });

  const auto [middle_height, leftover_rocks] = std::invoke([&]() {
    if (num_cycle_reps == 0) {
      return std::tuple{rock_index_t{0}, rock_index_t{0}};
    }
    const auto rocks_after_initial = num_rocks - initial_num_rocks;
    AOC_ASSERT(rocks_after_initial > 0,
               "Cannot have empty rocks after running cycles");
    AOC_ASSERT(rocks_extended_cycle > 0,
               "Cannot have empty rocks after running cycles");
    AOC_ASSERT(height_extended_cycle > 0,
               "Cannot have empty height after running cycles");

    const auto num_extended_cycles = rocks_after_initial / rocks_extended_cycle;
    const auto middle_height = num_extended_cycles * height_extended_cycle;
    const auto leftover_rocks = std::max(
        rock_index_t{0},
        rocks_after_initial - (rocks_extended_cycle * num_extended_cycles));
    return std::tuple{middle_height, leftover_rocks};
  });

  const rock_index_t leftover_height = std::invoke([&]() {
    const auto height_before = current_height - 1;
    for (int r = 0; r < leftover_rocks; ++r) {
      rock_solver(rock_index);
      ++rock_index;
    }
    return current_height - 1 - height_before;
  });

  std::cout << "  initial_height " << initial_height << std::endl;
  std::cout << "  middle_height " << middle_height << std::endl;
  std::cout << "  leftover_height " << leftover_height << std::endl;

  return initial_height + middle_height + leftover_height;
}

template <rock_index_t num_rocks>
rock_index_t solve_case(const std::string& filename) {
  std::vector<int> pattern;

  for (std::string line : aoc::views::read_lines(filename)) {
    std::ranges::transform(line, std::back_inserter(pattern), [](char value) {
      if (value == '<') {
        return -1;
      } else {
        return 1;
      }
    });
  }

  rock_index_t final_height = get_final_height<num_rocks>(pattern);
  std::cout << filename << " -> " << final_height << std::endl;
  return final_height;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(17, solve_case<10>("day17.example"));
  AOC_EXPECT_RESULT(3068, solve_case<2022>("day17.example"));
  AOC_EXPECT_RESULT(3085, solve_case<2022>("day17.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1514285714288, solve_case<1000000000000>("day17.example"));
  solve_case<1000000000000>("day17.input");
  AOC_RETURN_CHECK_RESULT();
}
