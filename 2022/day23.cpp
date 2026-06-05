// https://adventofcode.com/2022/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <limits>
#include <map>
#include <print>
#include <ranges>

namespace stdv = std::views;
namespace stdr = std::ranges;

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
  using elf_simulation_t = std::map<point, point>; // current -> proposed

  auto elf_simulation = elf_simulation_t{};
  for (let elf : elves) {
    elf_simulation[elf] = elf;
  }

  auto possible_proposals =
      aoc::as_consteval(std::array<std::array<aoc::facing_t, 3>, 4>{
          std::array{aoc::north, aoc::northeast, aoc::northwest},
          std::array{aoc::south, aoc::southwest, aoc::southeast},
          std::array{aoc::west, aoc::southwest, aoc::northwest},
          std::array{aoc::east, aoc::northeast, aoc::southeast},
      });

  let is_empty = [&](point pos, let& directions) {
    return stdr::all_of(
        directions | stdv::transform(aoc::get_diff<int>),
        [&](point diff) { return !elf_simulation.contains(pos + diff); });
  };

  let propose = [&](auto& entry) {
    auto& [current, proposed] = entry;
    proposed = current;
    if (is_empty(current, aoc::all_sky_directions)) {
      return;
    }
    for (let& directions : possible_proposals) {
      if (is_empty(current, directions)) {
        proposed = current + aoc::get_diff(directions[0]);
        break;
      }
    }
  };

  let execute = [&] {
    auto counts = aoc::flat_map<point, int>{};
    for (let& [ current, proposed ] : elf_simulation) {
      if (current != proposed) {
        ++counts[proposed];
      }
    }
    auto new_simulation = elf_simulation_t{};
    for (let& [ current, proposed ] : elf_simulation) {
      let dest = (counts[proposed] == 1) ? proposed : current;
      new_simulation[dest] = dest;
    }
    elf_simulation = std::move(new_simulation);
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
