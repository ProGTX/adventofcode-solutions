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
namespace stdr = std::ranges;

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

struct DijkstraLights {
  Lights lights;
  u32 decimal;

  constexpr DijkstraLights(Lights lights = {})
      : lights(std::move(lights)),
        decimal(aoc::binary_to_number(this->lights)) {}

  fn operator==(DijkstraLights const& other) const {
    return (decimal == other.decimal);
  }
  fn operator<=>(DijkstraLights const& other) const {
    return (decimal <=> other.decimal);
  }
};

fn solve_case1(Input const& input) -> u32 {
  return aoc::ranges::accumulate(
      input | stdv::transform([](let& tuple) {
        let & [ lights, buttons, _ ] = tuple;
        let start = DijkstraLights{lights |
                                   aoc::views::transform_to_value(0) |
                                   aoc::ranges::to<Lights>()};
        let target = DijkstraLights{lights};
        let distances = aoc::shortest_distances_dijkstra(
            start,
            [&](DijkstraLights const& current) {
              return //
                  buttons |
                  stdv::transform([&](Button const& button) {
                    auto next_lights = current.lights;
                    for (let pos : button) {
                      next_lights[pos] = static_cast<u8>(next_lights[pos] == 0);
                    }
                    return DijkstraLights{next_lights};
                  }) |
                  aoc::dijkstra_uniform_neighbors_view();
            },
            target);
        return static_cast<u32>(distances.at(target));
      }),
      u32{});
}

struct DijkstraJoltage {
  Joltage joltage;
  aoc::point_type<u64> compressed;
  static constexpr let max_jolt = u64{300};

  constexpr DijkstraJoltage(Joltage joltage = {})
      : joltage(std::move(joltage)) {
    // We can compress the joltage based on observed input
    auto multiplier = u64{1};
    for (let jolt : this->joltage | stdv::take(5)) {
      compressed.x += jolt * multiplier;
      multiplier *= max_jolt;
    }
    multiplier = 1;
    for (let jolt : this->joltage | stdv::drop(5)) {
      compressed.y += jolt * multiplier;
      multiplier *= max_jolt;
    }
  }

  fn increment(usize pos) {
    joltage[pos] += 1;
    auto* value = compressed.begin() + static_cast<usize>(pos >= 5);
    *value += aoc::pown(max_jolt, pos % 5);
  }

  fn operator==(DijkstraJoltage const& other) const {
    return compressed == other.compressed;
  }
  fn operator<=>(DijkstraJoltage const& other) const {
    return compressed <=> other.compressed;
  }
};

fn solve_case2(Input const& input) -> u64 {
  return aoc::ranges::accumulate(
      input | stdv::transform([](let& tuple) {
        let & [ _, buttons, joltage ] = tuple;
        let start = DijkstraJoltage{joltage |
                                    aoc::views::transform_to_value(0) |
                                    aoc::ranges::to<Joltage>()};
        let target = DijkstraJoltage{joltage};
        let distances = aoc::shortest_distances_dijkstra(
            start,
            [&](DijkstraJoltage const& current) {
              return //
                  buttons |
                  stdv::transform([&](Button const& button) {
                    auto next = current;
                    for (let pos : button) {
                      next.increment(pos);
                    }
                    return next;
                  }) |
                  stdv::filter([&](let& joltage) {
                    return stdr::all_of( //
                        joltage.joltage | stdv::enumerate, [&](let& pair) {
                          let[index, jolt] = pair;
                          return jolt <= target.joltage[index];
                        });
                  }) |
                  aoc::dijkstra_uniform_neighbors_view();
            },
            target);
        return static_cast<u64>(distances.at(target));
      }),
      u64{});
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  AOC_EXPECT_RESULT(7, solve_case1(example));
  let input = parse("day10.input");
  AOC_EXPECT_RESULT(535, solve_case1(input));

  std::println("Part 2");
  std::cout.flush();
  AOC_EXPECT_RESULT(33, solve_case2(example));
  // Dijkstra is too slow for part 2 :(
  // AOC_EXPECT_RESULT(100011612, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
