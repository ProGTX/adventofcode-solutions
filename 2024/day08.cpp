// https://adventofcode.com/2024/day/8

#include "../common/common.h"

#include <array>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

constexpr const char empty_space = '.';
using antennas_t = aoc::flat_map<char, aoc::static_vector<point, 4>>;

constexpr bool within_bounds(point check, const point& grid_size) {
  return (check.x >= 0) && (check.y >= 0) && (check.x < grid_size.x) &&
         (check.y < grid_size.y);
}

template <bool harmonics>
constexpr int calculate_antinodes(const antennas_t& antennas,
                                  const point& grid_size) {
  aoc::flat_set<point> antinodes;
  for (const auto& [antenna, locations] : antennas) {
    // Check every pair of locations
    for (const auto [loc_id, location1] : std::views::enumerate(locations)) {
      point anode;
      if constexpr (harmonics) {
        if (locations.size() > 1) {
          antinodes.insert(location1);
        }
      }
      for (const auto location2 : locations | std::views::drop(loc_id + 1)) {
        const auto distance12 = location2 - location1;
        anode = location1;
        while (true) {
          anode -= distance12;
          if (!within_bounds(anode, grid_size)) {
            break;
          }
          antinodes.insert(anode);
          if constexpr (!harmonics) {
            break;
          }
        }
        anode = location2;
        while (true) {
          anode += distance12;
          if (!within_bounds(anode, grid_size)) {
            break;
          }
          antinodes.insert(anode);
          if constexpr (!harmonics) {
            break;
          }
        }
      }
    }
  }
  return antinodes.size();
}

template <bool harmonics>
int solve_case(const std::string& filename) {
  point grid_size{0, 0};
  antennas_t antennas;

  for (int& row = grid_size.y;
       std::string_view line : aoc::views::read_lines(filename)) {
    if (grid_size.x == 0) {
      grid_size.x = line.size();
    }
    for (auto [column, value] : std::views::enumerate(line)) {
      if (value != empty_space) {
        antennas[value].push_back({static_cast<int>(column), row});
      }
    }
    ++row;
  }

  auto sum = calculate_antinodes<harmonics>(antennas, grid_size);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(14, solve_case<false>("day08.example"));
  AOC_EXPECT_RESULT(220, solve_case<false>("day08.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(34, solve_case<true>("day08.example"));
  AOC_EXPECT_RESULT(813, solve_case<true>("day08.input"));
  AOC_RETURN_CHECK_RESULT();
}
