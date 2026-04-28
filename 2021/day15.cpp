// https://adventofcode.com/2021/day/15

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <print>
#include <ranges>
#include <string>

using Input = aoc::char_grid<>;

auto parse(String const& filename) -> Input {
  return aoc::read_char_grid(filename);
}

fn solve_case1(Input const& grid) -> int {
  let start = point{0, 0};
  let end = point{static_cast<int>(grid.num_columns()) - 1,
                  static_cast<int>(grid.num_rows()) - 1};
  let distances = aoc::shortest_distances_dijkstra(
      start,
      [&](point current) {
        return grid.basic_neighbor_positions(current) |
               std::views::transform([&](point neighbor) {
                 return aoc::dijkstra_neighbor_t{
                     neighbor, grid.at(neighbor.y, neighbor.x) - '0'};
               });
      },
      end);
  return distances.find(end)->second;
}

fn virtual_cost(Input const& grid, point pos) -> int {
  let base = grid.at(pos.y % grid.num_rows(), pos.x % grid.num_columns()) - '0';
  let offset = pos.y / static_cast<int>(grid.num_rows()) +
               pos.x / static_cast<int>(grid.num_columns());
  return (base + offset - 1) % 9 + 1;
}

fn virtual_neighbors(point pos, int rows, int cols)
    -> aoc::static_vector<point, 4> {
  auto neighbors = aoc::static_vector<point, 4>{};
  if (pos.y > 0) {
    neighbors.push_back({pos.x, pos.y - 1});
  }
  if (pos.x > 0) {
    neighbors.push_back({pos.x - 1, pos.y});
  }
  if (pos.y + 1 < rows) {
    neighbors.push_back({pos.x, pos.y + 1});
  }
  if (pos.x + 1 < cols) {
    neighbors.push_back({pos.x + 1, pos.y});
  }
  return neighbors;
}

fn solve_case2(Input const& grid) -> int {
  let rows = static_cast<int>(grid.num_rows()) * 5;
  let cols = static_cast<int>(grid.num_columns()) * 5;
  let start = point{0, 0};
  let end = point{cols - 1, rows - 1};
  let distances = aoc::shortest_distances_dijkstra(
      start,
      [&](point current) {
        return virtual_neighbors(current, rows, cols) |
               std::views::transform([&](point neighbor) {
                 return aoc::dijkstra_neighbor_t{neighbor,
                                                 virtual_cost(grid, neighbor)};
               });
      },
      end);
  return distances.find(end)->second;
}

int main() {
  std::println("Part 1");
  let example = parse("day15.example");
  AOC_EXPECT_RESULT(40, solve_case1(example));
  let input = parse("day15.input");
  AOC_EXPECT_RESULT(592, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(315, solve_case2(example));
  AOC_EXPECT_RESULT(2897, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
