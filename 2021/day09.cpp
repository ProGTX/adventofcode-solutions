// https://adventofcode.com/2021/day/9

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using heightmap_t = aoc::char_grid<>;
constexpr const char wall = '9';

constexpr std::vector<point> find_low_points(const heightmap_t& heightmap) {
  std::vector<point> low_points;
  for (int row = 0; row < heightmap.num_rows(); ++row) {
    for (int col = 0; col < heightmap.num_columns(); ++col) {
      const auto current_point = point{col, row};
      const auto current_height = heightmap.at(row, col);
      if (std::ranges::all_of(
              heightmap.basic_neighbor_values({col, row}),
              [&](const char neighbor) { return current_height < neighbor; })) {
        low_points.push_back(current_point);
      }
    }
  }
  return low_points;
}

constexpr int sum_risk_levels(const heightmap_t& heightmap) {
  const auto low_points = find_low_points(heightmap);
  return aoc::ranges::accumulate(
      low_points | std::views::transform([&](const point lp) {
        return aoc::to_number<int>(heightmap.at(lp.y, lp.x)) + 1;
      }),
      0);
}

constexpr int find_basin(const heightmap_t& heightmap, const point low_point) {
  aoc::flat_set<point> basin;
  aoc::flat_set<point> unvisited;
  unvisited.emplace(low_point);
  while (!unvisited.empty()) {
    auto current_it = unvisited.end() - 1;
    const auto current = *current_it;
    unvisited.erase(current_it);
    basin.emplace(current);
    for (const auto neighbor : heightmap.basic_neighbor_positions(current)) {
      if (basin.contains(neighbor) ||
          (heightmap.at(neighbor.y, neighbor.x) == wall)) {
        continue;
      }
      unvisited.emplace(neighbor);
    }
  }
  return basin.size();
}

constexpr int largest_basins(const heightmap_t& heightmap) {
  const auto low_points = find_low_points(heightmap);
  std::vector<int> basins;
  std::ranges::transform(
      low_points, aoc::inserter_it(basins),
      [&](const point lp) { return find_basin(heightmap, lp); });
  std::ranges::partial_sort(basins, std::begin(basins) + 3, std::greater{});
  return std::ranges::fold_left(std::span{basins}.subspan(0, 3), 1,
                                std::multiplies{});
}

template <bool basin>
int solve_case(const std::string& filename) {
  heightmap_t heightmap = aoc::read_char_grid(filename);

  int sum = 0;
  if constexpr (!basin) {
    sum = sum_risk_levels(heightmap);
  } else {
    sum = largest_basins(heightmap);
  }
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(15, solve_case<false>("day09.example"));
  AOC_EXPECT_RESULT(580, solve_case<false>("day09.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1134, solve_case<true>("day09.example"));
  AOC_EXPECT_RESULT(856716, solve_case<true>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
