// https://adventofcode.com/2022/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <limits>
#include <print>
#include <ranges>

using elf_t = point;
using elves_t = Vec<elf_t>;

fn parse(String const& filename) -> elves_t {
  auto elves = elves_t{};
  for (int linenum = 1; str line : aoc::views::read_lines(filename)) {
    let row = linenum - 1;
    for (int column = 0; char value : line) {
      if (value == '#') {
        elves.push_back(point{column, row});
      }
      ++column;
    }
    ++linenum;
  }
  return elves;
}

fn simulate(elves_t& elves, int num_rounds) -> int {
  // current -> proposed
  using elf_simulation_t = aoc::hash_map<point, point>;

  auto elf_simulation = elf_simulation_t{};
  for (let elf : elves) {
    elf_simulation[elf] = elf;
  }

  // Offsets rather than facings: the rotation below reorders these every round,
  // so a facing here would be decoded through get_diff's switch on every one of
  // the millions of neighbor tests, rather than once at startup.
  // Each entry leads with the direction the elf actually moves in.
  let diffs = [](let... facings) {
    return std::array{aoc::get_diff<int>(facings)...};
  };
  auto possible_proposals = std::array{
      diffs(aoc::north, aoc::northeast, aoc::northwest),
      diffs(aoc::south, aoc::southwest, aoc::southeast),
      diffs(aoc::west, aoc::southwest, aoc::northwest),
      diffs(aoc::east, aoc::northeast, aoc::southeast),
  };
  constexpr let all_sky_diffs = [] {
    auto result = std::array<point, aoc::all_sky_directions.size()>{};
    for (usize i = 0; i < result.size(); ++i) {
      result[i] = aoc::get_diff<int>(aoc::all_sky_directions[i]);
    }
    return result;
  }();

  let is_empty = [&](point pos, let& direction_diffs) {
    return stdr::all_of(direction_diffs, [&](point diff) {
      return !elf_simulation.contains(pos + diff);
    });
  };

  let propose = [&](auto& entry) {
    auto& [current, proposed] = entry;
    proposed = current;
    if (is_empty(current, all_sky_diffs)) {
      return;
    }
    for (let& direction_diffs : possible_proposals) {
      if (is_empty(current, direction_diffs)) {
        proposed = current + direction_diffs[0];
        break;
      }
    }
  };

  auto counts = aoc::hash_map<point, int>{};
  auto new_simulation = elf_simulation_t{};
  let execute = [&] {
    // Reused across rounds: the elf count never changes,
    // so clear() keeps the buckets both maps already grew into
    counts.clear();
    new_simulation.clear();
    for (let& [ current, proposed ] : elf_simulation) {
      if (current != proposed) {
        ++counts[proposed];
      }
    }
    for (let& [ current, proposed ] : elf_simulation) {
      // find instead of operator[], which would enter every stationary elf's
      // own position into counts and grow it past the elves that actually move
      let it = counts.find(proposed);
      let dest =
          ((it != counts.end()) && (it->second == 1)) ? proposed : current;
      new_simulation[dest] = dest;
    }
    // swap instead of move-assign, so next round's clear() finds the buckets
    // this round already grew into
    elf_simulation.swap(new_simulation);
  };

  int round = 0;
  for (; round < num_rounds; ++round) {
    stdr::for_each(elf_simulation, propose);
    if (stdr::all_of(elf_simulation,
                     [](let& p) { return p.first == p.second; })) {
      break;
    }
    execute();
    aoc::ranges::rotate_left(possible_proposals);
  }

  elves.clear();
  for (let& [ pos, _ ] : elf_simulation) {
    elves.push_back(pos);
  }
  return round + 1;
}

fn solve_case1(elves_t elves) -> int {
  (void)simulate(elves, 10);
  let field_size = aoc::min_max_helper::get(elves).grid_size();
  return (field_size.x * field_size.y) - static_cast<int>(elves.size());
}

fn solve_case2(elves_t elves) -> int {
  return simulate(elves, std::numeric_limits<int>::max());
}

int main() {
  std::println("Part 1");
  let example2 = parse("day23.example2");
  AOC_EXPECT_RESULT(25, solve_case1(example2));
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(110, solve_case1(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(3906, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4, solve_case2(example2));
  AOC_EXPECT_RESULT(20, solve_case2(example));
  AOC_EXPECT_RESULT(895, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
