// https://adventofcode.com/2021/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <print>
#include <ranges>
#include <set>

using EnergyLevels = aoc::char_grid<>;
constexpr let usize_max = std::numeric_limits<usize>::max();

fn parse(String const& filename) -> EnergyLevels {
  return aoc::read_char_grid(filename);
}

template <usize NUM_STEPS>
fn solve_case(EnergyLevels const& energy_levels) -> usize {
  auto current_levels = energy_levels;
  auto sum = 0;
  for (let step : Range{0uz, NUM_STEPS}) {
    auto unvisited = Vec<usize>{};
    auto visited = std::set<usize>{};

    // Increase level by 1
    for (auto&& [index, energy] : current_levels | std::views::enumerate) {
      energy += 1;
      if (energy > '9') {
        unvisited.push_back(index);
      }
    }

    // Flash
    while (!unvisited.empty()) {
      let current_index = aoc::pop_stack(unvisited);
      if (let[_, inserted] = visited.insert(current_index); !inserted) {
        continue;
      }
      for (let neighbor_pos : current_levels.all_neighbor_positions(
               current_levels.position(current_index))) {
        let index = current_levels.linear_index(neighbor_pos);
        auto& energy = current_levels.at_index(index);
        energy += 1;
        if ((energy > '9') && !visited.contains(index)) {
          unvisited.push_back(index);
        }
      }
    }

    // Reset flashed
    for (let index : visited) {
      current_levels.modify('0', index);
    }

    if constexpr (NUM_STEPS < usize_max) {
      sum += visited.size();
    } else if (visited.size() == current_levels.size()) {
      return step + 1;
    }
  }
  return sum;
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(9, solve_case<2>(example));
  let example2 = parse("day11.example2");
  AOC_EXPECT_RESULT(1656, solve_case<100>(example2));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(1694, solve_case<100>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_case<usize_max>(example));
  AOC_EXPECT_RESULT(195, solve_case<usize_max>(example2));
  AOC_EXPECT_RESULT(346, solve_case<usize_max>(input));

  AOC_RETURN_CHECK_RESULT();
}
