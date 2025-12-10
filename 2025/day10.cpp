// https://adventofcode.com/2025/day/10

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <tuple>

using Lights = aoc::static_vector<u8, 10>;
using Button = aoc::static_vector<usize, 9>;
using Buttons = aoc::static_vector<Button, 13>;
using Joltage = aoc::static_vector<u16, 10>;
using Input = Vec<std::tuple<Lights, Buttons, Joltage>>;

namespace stdv = std::views;

fn parse(String const& filename) -> Input {
  using tuple = Input::value_type;
  return //
      aoc::views::read_lines(filename) |
      stdv::transform([](str line) {
        let[lights, rest] = aoc::split_once(line, "] (");
        let[buttons, joltage] = aoc::split_once(rest, ") {");
        return tuple{
            lights.substr(1) | stdv::transform([](char c) {
              return static_cast<u8>(c == '#');
            }) | aoc::ranges::to<Lights>(),
            aoc::split(buttons, ") (") | stdv::transform([](str button) {
              return aoc::split<Button>(button, ',');
            }) | aoc::ranges::to<Buttons>(),
            aoc::split<Joltage>(joltage.substr(0, joltage.size() - 1), ',')};
      }) |
      aoc::ranges::to<Input>();
}

struct DijkstraState {
  Lights lights;
  u32 decimal;

  constexpr DijkstraState(Lights lights = {})
      : lights(std::move(lights)), decimal(aoc::binary_to_number(lights)) {}

  fn operator==(DijkstraState const& other) const {
    return (decimal == other.decimal);
  }
  fn operator<=>(DijkstraState const& other) const {
    return (decimal <=> other.decimal);
  }
};

fn solve_case1(Input const& input) -> u32 {
  using DijkstraNeighbors =
      aoc::static_vector<aoc::dijkstra_neighbor_t<DijkstraState>,
                         Buttons{}.capacity()>;
  return aoc::ranges::accumulate(
      input | stdv::transform([](let& tuple) {
        let & [ lights, buttons, _ ] = tuple;
        let start = DijkstraState{lights |
                                  aoc::views::transform_to_value(0) |
                                  aoc::ranges::to<Lights>()};
        let target = DijkstraState{lights};
        let distances = aoc::shortest_distances_dijkstra(
            start,
            [&](DijkstraState const& current) {
              return //
                  buttons |
                  stdv::transform([&](Button const& button) {
                    auto next_lights = current.lights;
                    for (let pos : button) {
                      next_lights[pos] = static_cast<u8>(next_lights[pos] == 0);
                    }
                    return DijkstraState{next_lights};
                  }) |
                  aoc::dijkstra_uniform_neighbors_view() |
                  aoc::ranges::to<DijkstraNeighbors>();
            },
            target);
        return static_cast<u32>(distances.at(target));
      }),
      u32{});
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  AOC_EXPECT_RESULT(7, solve_case1(example));
  let input = parse("day10.input");
  AOC_EXPECT_RESULT(535, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(33, solve_case2(example));
  // AOC_EXPECT_RESULT(100011612, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
