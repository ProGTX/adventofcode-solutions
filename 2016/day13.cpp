// https://adventofcode.com/2016/day/13

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <bit>
#include <limits>
#include <print>
#endif

constexpr let START = point{1, 1};

/// The office designer's favorite number, and the position to reach
struct Input {
  u32 favorite;
  point target;
};

auto parse(String const& filename) -> Input {
  let favorite = aoc::to_number<u32>(aoc::trim(aoc::read_file(filename)));
  // The example is a smaller maze, with a closer target
  let target = (favorite == 10) ? point{7, 4} : point{31, 39};
  return Input{favorite, target};
}

/// Whether the position holds an open space rather than a wall
fn is_space(point pos, u32 favorite) -> bool {
  let x = static_cast<u32>(pos.x);
  let y = static_cast<u32>(pos.y);
  let value = (x * x) + (3 * x) + (2 * x * y) + y + (y * y) + favorite;
  return (std::popcount(value) % 2) == 0;
}

/// Anything further than `max_steps` away from the start
/// can never be reached in time,
/// which is what keeps a search of this endless maze finite
fn get_neighbors(point current, u32 favorite, int max_steps) {
  return aoc::basic_neighbor_diffs |
         stdv::transform([current](point diff) { return current + diff; }) |
         stdv::filter([=](point neighbor) {
           // Only non-negative coordinates exist
           return (neighbor.x >= 0) &&
                  (neighbor.y >= 0) &&
                  (distance_manhattan(neighbor, START) <= max_steps) &&
                  is_space(neighbor, favorite);
         }) |
         aoc::dijkstra_uniform_neighbors_view();
}

fn solve_case1(Input const& input) -> u32 {
  let[favorite, target] = input;
  let distances = aoc::shortest_distances_astar(
      START, //
      target,
      [&](point current) {
        return get_neighbors(current, favorite,
                             std::numeric_limits<int>::max());
      },
      [&](point current) { return distance_manhattan(current, target); });
  return static_cast<u32>(distances.at(target));
}

fn solve_case2(Input const& input) -> usize {
  constexpr let MAX_STEPS = 50;
  let distances = aoc::shortest_distances_dijkstra(START, [&](point current) {
    return get_neighbors(current, input.favorite, MAX_STEPS);
  });
  // Every place the search settled, that is close enough to walk to
  return static_cast<usize>(stdr::count_if(
      distances, [](let& entry) { return entry.second <= MAX_STEPS; }));
}

int main() {
  std::println("Part 1");
  let example = parse("day13.example");
  AOC_EXPECT_RESULT(11, solve_case1(example));
  let input = parse("day13.input");
  AOC_EXPECT_RESULT(86, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(151, solve_case2(example));
  AOC_EXPECT_RESULT(127, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
