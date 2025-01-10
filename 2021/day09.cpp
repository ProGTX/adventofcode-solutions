// https://adventofcode.com/2021/day/9

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using heightmap_t = aoc::char_grid<>;

constexpr std::vector<point> find_low_points(const heightmap_t& heightmap) {
  std::vector<point> low_points;
  for (int row = 0; row < heightmap.num_rows(); ++row) {
    for (int col = 0; col < heightmap.num_columns(); ++col) {
      const auto current_point = point{col, row};
      const auto current_height = heightmap.at(row, col);
      if (std::ranges::all_of(
              heightmap.basic_neighbors({col, row}), [&](const point neighbor) {
                return current_height < heightmap.at(neighbor.y, neighbor.x);
              })) {
        low_points.push_back(current_point);
      }
    }
  }
  return low_points;
}

constexpr int sum_risk_levels(const heightmap_t& heightmap,
                              std::span<const point> low_points) {
  return aoc::ranges::accumulate(
      low_points | std::views::transform([&](const point lp) {
        return aoc::to_number<int>(heightmap.at(lp.y, lp.x)) + 1;
      }),
      0);
}

template <bool>
int solve_case(const std::string& filename) {
  heightmap_t heightmap = aoc::read_char_grid(filename);

  int sum = 0;
  sum = sum_risk_levels(heightmap, find_low_points(heightmap));
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(15, solve_case<false>("day09.example"));
  AOC_EXPECT_RESULT(580, solve_case<false>("day09.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day09.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
