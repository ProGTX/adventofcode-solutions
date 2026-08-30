// https://adventofcode.com/2016/day/24

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <utility>
#endif

using Maze = aoc::char_grid<>;

/// The maze, and the locations of the numbered locations
using Input = std::pair<Maze, Vec<point>>;

constexpr let WALL = '#';
constexpr let OPEN = '.';

fn parse(String const& filename) -> Input {
  auto maze = aoc::read_char_grid(filename);
  auto numbers = Vec<std::pair<u32, usize>>{};
  for (let index : Range{0uz, maze.size()}) {
    auto& tile = maze.at_index(index);
    if ((tile >= '0') && (tile <= '9')) {
      numbers.emplace_back(aoc::to_number<u32>(tile), index);
      // The numbers are just markers, the maze is open at their locations
      tile = OPEN;
    }
  }
  stdr::sort(numbers);
  auto locations = numbers |
                   stdv::transform([&](let& number) {
                     return maze.position(number.second);
                   }) |
                   aoc::collect_vec<point>();
  return Input{std::move(maze), std::move(locations)};
}

/// The number of steps between every pair of numbered locations.
///
/// One search per location, over the open tiles of the maze,
/// which settles the distance to every other location at once.
fn location_distances(Input const& input) -> aoc::grid<u32> {
  let & [ maze, locations ] = input;
  let num_locations = locations.size();
  auto distances = aoc::grid<u32>{0u, num_locations, num_locations};
  for (let[from_id, from_pos] : locations | stdv::enumerate) {
    let steps = aoc::shortest_distances_dijkstra(from_pos, [&](point current) {
      return maze.basic_neighbor_positions(current) |
             stdv::filter([&](point neighbor) {
               return maze.at(neighbor.y, neighbor.x) != WALL;
             }) |
             aoc::dijkstra_uniform_neighbors_view();
    });
    for (let[to_id, to_pos] : locations | stdv::enumerate) {
      distances.modify(static_cast<u32>(steps.at(to_pos)),
                       static_cast<usize>(from_id), static_cast<usize>(to_id));
    }
  }
  return distances;
}

/// The fewest steps needed to visit every numbered location,
/// starting from location 0,
/// optionally returning back to it at the end.
template <bool ROUND_TRIP>
fn solve_case(Input const& input) -> u32 {
  // Only the longest route is searched for directly.
  // Every complete route visits every location,
  // so it always has exactly `num_locations - 1` legs -
  // which means the shortest route is the longest one
  // over the complemented weights (max_steps - steps),
  // and its real length is `(num_locations - 1) * max_steps`
  // minus what's found.
  let distances = location_distances(input);
  let num_locations = distances.num_rows();
  let max_steps = *stdr::max_element(distances.data());
  // The way back home depends on where the route ends,
  // so every location is tried as the last one,
  // which keeps each search a simple path of `num_locations - 1` legs.
  // A one-way route doesn't care where it ends, so one search is enough.
  let last_locations = ROUND_TRIP ? num_locations : 1uz;
  return stdr::min(
      aocv::indices(last_locations) |
      aocv::transform_filter([&](usize last) -> Option<u32> {
        let longest = aoc::longest_simple_path(
            num_locations, //
            0uz,
            [&](usize location, usize num_visited) {
              return (num_visited == num_locations) &&
                     (!ROUND_TRIP || (location == last));
            },
            [&](usize location) {
              return aocv::indices(num_locations) |
                     stdv::transform([&, location](usize to_id) {
                       return aoc::dijkstra_neighbor_t<usize>{
                           to_id,
                           static_cast<int>(max_steps -
                                            distances.at(location, to_id))};
                     });
            });
        if (!longest.has_value()) {
          return None;
        }
        let route = (static_cast<u32>(num_locations) - 1) * max_steps -
                    static_cast<u32>(*longest);
        let way_back = ROUND_TRIP ? distances.at(last, 0) : 0u;
        return route + way_back;
      }));
}

constexpr let ONE_WAY = false;
constexpr let ROUND_TRIP = true;

int main() {
  std::println("Part 1");
  let example = parse("day24.example");
  AOC_EXPECT_RESULT(14, solve_case<ONE_WAY>(example));
  let input = parse("day24.input");
  AOC_EXPECT_RESULT(500, solve_case<ONE_WAY>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(20, solve_case<ROUND_TRIP>(example));
  AOC_EXPECT_RESULT(748, solve_case<ROUND_TRIP>(input));

  AOC_RETURN_CHECK_RESULT();
}
