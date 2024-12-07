// https://adventofcode.com/2023/day/21

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::uint64_t;
using garden_t = aoc::char_grid<>;
using positions_t = std::vector<point>;

constexpr inline auto plot = '.';
constexpr inline auto rock = '#';

/**
 * This function performs a single step of movement for the pots in the garden.
 *
 * @tparam infinite If true, the garden is considered infinite, meaning that if
 * a pot moves off the edge of the garden, it wraps around to the other side.
 * @param garden    The garden containing the pots.
 * @param starting   The starting positions of the pots.
 * @return           The final positions of the pots after the movement step.
 */
template <bool infinite>
positions_t move_one(const garden_t& garden, const positions_t& starting) {
  // Create a vector to store the final positions of the pots.
  positions_t ending;

  // Get the dimensions of the garden.
  const auto bounds = point(garden.num_columns(), garden.num_rows());

  // Loop over each starting pot.
  for (auto start : starting) {
    // Loop over each direction a pot can move.
    for (auto diff : aoc::basic_neighbor_diffs) {
      // Calculate the position of the neighboring pot.
      point neighbor = start + diff;

      // Check if the garden is not infinite.
      if constexpr (!infinite) {
        // If the garden is not infinite, check if the neighboring pot is in
        // bounds and is a plot. If so, add it to the final positions.
        if (garden.in_bounds(neighbor.y, neighbor.x) &&
            (garden.at(neighbor.y, neighbor.x) == plot)) {
          ending.push_back(neighbor);
        }
      } else {
        // If the garden is infinite, calculate the position of the neighboring
        // pot taking into account wrapping around the edges. Then check if the
        // neighboring pot is a plot. If so, add it to the final positions.
        auto neighbor_in_bounds = ((neighbor % bounds) + bounds) % bounds;
        if (garden.at(neighbor_in_bounds.y, neighbor_in_bounds.x) == plot) {
          ending.push_back(neighbor);
        }
      }
    }
  }

  // Sort the final positions in ascending order.
  std::ranges::sort(ending, std::less<>{});

  // Remove any duplicate positions.
  auto [non_unique_begin, non_unique_end] = std::ranges::unique(ending);
  ending.resize(std::distance(std::begin(ending), non_unique_begin));

  // Return the final positions of the pots.
  return ending;
}

template <int num_steps, bool infinite>
int_t solve_case(const std::string& filename) {
  std::cout << filename << " " << num_steps << " " << infinite << std::endl;

  garden_t garden;
  positions_t positions;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    garden.add_row(line);
    auto start_pos = line.find("S");
    if (start_pos != std::string::npos) {
      auto start = point(start_pos, garden.num_rows() - 1);
      positions.push_back(start);
      garden.modify(plot, start.y, start.x);
    }
  }

  for (int i = 0; i < num_steps; ++i) {
    positions = move_one<infinite>(garden, positions);
  }

  int_t sum = positions.size();
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(16, (solve_case<6, false>("day21.example")));
  AOC_EXPECT_RESULT(3646, (solve_case<64, false>("day21.input")));
  // std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(16, (solve_case<6, true>("day21.example")));
  AOC_EXPECT_RESULT(50, (solve_case<10, true>("day21.example")));
  AOC_EXPECT_RESULT(1594, (solve_case<50, true>("day21.example")));
  AOC_EXPECT_RESULT(6536, (solve_case<100, true>("day21.example")));
  AOC_EXPECT_RESULT(167004, (solve_case<500, true>("day21.example")));
  AOC_EXPECT_RESULT(668697, (solve_case<1000, true>("day21.example")));
  // AOC_EXPECT_RESULT(16733044, (solve_case<5000, true>("day21.example")));
  // AOC_EXPECT_RESULT(90111113594927, (solve_case<26501365,
  // true>("day21.input")));
  AOC_RETURN_CHECK_RESULT();
}
