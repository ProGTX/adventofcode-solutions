// https://adventofcode.com/2023/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>

using garden_t = aoc::char_grid<>;
using positions_t = Vec<point>;

constexpr let plot = '.';
constexpr let rock = '#';

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
fn move_one(positions_t& ending, garden_t const& garden,
            positions_t const& starting) -> positions_t {
  // Create a vector to store the final positions of the pots.
  ending.clear();
  // Get the dimensions of the garden.
  let bounds = point(garden.num_columns(), garden.num_rows());

  // Loop over each starting pot.
  for (let start : starting) {
    // Loop over each direction a pot can move.
    for (let diff : aoc::basic_neighbor_diffs) {
      // Calculate the position of the neighboring pot.
      let neighbor = start + diff;

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
        let neighbor_in_bounds = ((neighbor % bounds) + bounds) % bounds;
        if (garden.at(neighbor_in_bounds.y, neighbor_in_bounds.x) == plot) {
          ending.push_back(neighbor);
        }
      }
    }
  }

  // Sort the final positions in ascending order.
  stdr::sort(ending);
  // Remove any duplicate positions.
  let[non_unique_begin, non_unique_end] = stdr::unique(ending);
  ending.resize(static_cast<usize>(non_unique_begin - ending.begin()));

  // Return the final positions of the pots.
  return std::move(ending);
}

struct input_t {
  garden_t garden;
  positions_t positions;
};

fn parse(String const& filename) -> input_t {
  garden_t garden;
  positions_t positions;

  for (str line : aoc::views::read_lines(filename)) {
    garden.add_row(line);
    let start_pos = line.find('S');
    if (start_pos != str::npos) {
      let start = point(start_pos, garden.num_rows() - 1);
      positions.push_back(start);
      garden.modify(plot, start.y, start.x);
    }
  }

  return input_t{std::move(garden), std::move(positions)};
}

template <int num_steps, bool infinite>
fn solve_case(input_t input) -> u64 {
  auto ending = positions_t{};
  for (i32 i = 0; i < num_steps; ++i) {
    input.positions = move_one<infinite>(ending, input.garden, input.positions);
  }

  return input.positions.size();
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(16, (solve_case<6, false>(example)));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(3646, (solve_case<64, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(16, (solve_case<6, true>(example)));
  AOC_EXPECT_RESULT(50, (solve_case<10, true>(example)));
  AOC_EXPECT_RESULT(1594, (solve_case<50, true>(example)));
  AOC_EXPECT_RESULT(6536, (solve_case<100, true>(example)));
  aoc::return_incomplete();
  AOC_EXPECT_RESULT(167004, (solve_case<500, true>(example)));
  AOC_EXPECT_RESULT(668697, (solve_case<1000, true>(example)));
  AOC_EXPECT_RESULT(16733044, (solve_case<5000, true>(example)));
  AOC_EXPECT_RESULT(1337, (solve_case<26501365, true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
