// https://adventofcode.com/2022/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Terminator = aoc::closed_range<point>;
using Input = std::pair<aoc::char_grid<>, Terminator>;

namespace stdv = std::views;
namespace stdr = std::ranges;

fn parse(String const& filename) -> Input {
  auto [heightmap, config] =
      aoc::read_char_grid(filename, aoc::char_grid_config_input{
                                        .padding = {},
                                        .start_char = 'S',
                                        .end_char = 'E',
                                    });
  // Replace begin and end with regular heights
  let terminator = Terminator{config.start_pos.value(), config.end_pos.value()};
  heightmap.modify('a', terminator.begin.y, terminator.begin.x);
  heightmap.modify('z', terminator.end.y, terminator.end.x);
  return {std::move(heightmap), terminator};
}

fn shortest_path(aoc::char_grid<> const& heightmap, point start, point end)
    -> Option<u32> {
  let distances = aoc::shortest_distances_dijkstra(
      start,
      [&](point current) {
        return heightmap.basic_neighbor_positions(current) |
               stdv::filter([&, current_height = heightmap.at(
                                    current.y, current.x)](point pos) {
                 let neighbor = heightmap.at(pos.y, pos.x);
                 return neighbor <= (current_height + 1);
               }) |
               aoc::dijkstra_uniform_neighbors_view();
      },
      end);
  let it = distances.find(end);
  if (it == std::end(distances)) {
    return {};
  }
  return it->second;
}

fn solve_case1(Input const& input) -> u32 {
  let & [ heightmap, terminator ] = input;
  return shortest_path(heightmap, terminator.begin, terminator.end).value();
}

fn solve_case2(Input const& input) -> u32 {
  let & [ heightmap, terminator ] = input;
  return stdr::min( //
      heightmap |
      stdv::enumerate |
      stdv::filter([](let& current) {
        let & [ _, value ] = current;
        return value == 'a';
      }) |
      stdv::transform([&](let& current) {
        let & [ linear_index, _ ] = current;
        return shortest_path(heightmap, heightmap.position(linear_index),
                             terminator.end)
            .value_or(heightmap.size());
      }));
}

int main() {
  std::println("Part 1");
  let example = parse("day12.example");
  AOC_EXPECT_RESULT(31, solve_case1(example));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(504, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(29, solve_case2(example));
  AOC_EXPECT_RESULT(500, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
