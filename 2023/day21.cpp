// https://adventofcode.com/2023/day/21

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using int_t = int;
using garden_t = aoc::char_grid<>;

constexpr inline auto plot = '.';
constexpr inline auto rock = '#';

std::set<point> move_one(const garden_t& garden,
                         const std::set<point>& starting) {
  std::set<point> ending;
  for (auto start : starting) {
    for (auto diff : aoc::basic_neighbor_diffs) {
      point neighbor = start + diff;
      if (garden.in_bounds(neighbor.y, neighbor.x) &&
          (garden.at(neighbor.y, neighbor.x) == plot)) {
        ending.insert(neighbor);
      }
    }
  }
  return ending;
}

template <int num_steps>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  garden_t garden;
  std::set<point> positions;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    garden.add_row(line);
    auto start_pos = line.find("S");
    if (start_pos != std::string::npos) {
      auto start = point(start_pos, garden.num_rows() - 1);
      positions.insert(start);
      garden.modify(plot, start.y, start.x);
    }
  }

  for (int i = 0; i < num_steps; ++i) {
    positions = move_one(garden, positions);
  }

  int_t sum = positions.size();
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(16, (solve_case<6>("day21.example")));
  AOC_EXPECT_RESULT(3646, (solve_case<64>("day21.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(952408144115, (solve_case<true>("day21.example")));
  // AOC_EXPECT_RESULT(90111113594927, (solve_case<true>("day21.input")));
  AOC_RETURN_CHECK_RESULT();
}
