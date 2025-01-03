// https://adventofcode.com/2022/day/12

#include "../common/compiler.h"

#if defined(AOC_COMPILER_MSVC)
// WORKAROUND
#define AOC_DISABLE_MODULES
#endif

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct heightmap_t : public aoc::grid<int> {
  point begin_pos;
  point end_pos;
};

template <bool reverse>
constexpr int get_fewest_steps(const heightmap_t& heightmap) {
  const point start_node = reverse ? heightmap.end_pos : heightmap.begin_pos;

  std::vector<point> end_nodes;
  if constexpr (!reverse) {
    end_nodes.push_back(heightmap.end_pos);
  } else {
    for (int row = 0; row < heightmap.num_rows(); ++row) {
      for (int col = 0; col < heightmap.num_columns(); ++col) {
        if (heightmap.at(row, col) == 0) {
          end_nodes.push_back(point{col, row});
        }
      }
    }
  }

  const auto find_neighbors = [&](const point current_pos) {
    auto neighbors = aoc::static_vector<aoc::dijkstra_neighbor_t<point>, 4>{};
    const auto current_height = heightmap.at(current_pos.y, current_pos.x);
    for (const auto direction : aoc::basic_sky_directions) {
      const auto neighbor = current_pos + aoc::get_diff(direction);
      if (!heightmap.in_bounds(neighbor.y, neighbor.x)) {
        continue;
      }

      auto height_diff = heightmap.at(neighbor.y, neighbor.x) - current_height;
      // Cannot climb very high, but can drop a lot
      if constexpr (reverse) {
        if (height_diff < -1) {
          continue;
        }
      } else {
        if (height_diff > 1) {
          continue;
        }
      }
      neighbors.emplace_back(neighbor, 1);
    }
    return neighbors;
  };

  const auto distances =
      aoc::shortest_distances_dijkstra(start_node, end_nodes, find_neighbors);

  int shortest_path = 1 << 30;
  for (const auto end_node : end_nodes) {
    auto it = distances.find(end_node);
    if (it != std::end(distances)) {
      shortest_path = std::min(shortest_path, it->second);
    }
  }
  return shortest_path;
}

template <bool reverse>
int solve_case(const std::string& filename) {
  using row_t = typename heightmap_t::row_t;
  heightmap_t heightmap;
  row_t row;

  for (std::string line : aoc::views::read_lines(filename)) {
    row.clear();
    row.reserve(line.size());
    int column = 0;
    std::ranges::transform(line, std::back_inserter(row), [&](char value) {
      switch (value) {
        case 'S':
          heightmap.begin_pos = point(column, heightmap.num_rows());
          ++column;
          return 0;
        case 'E':
          heightmap.end_pos = point(column, heightmap.num_rows());
          ++column;
          return static_cast<int>('z' - 'a');
        default:
          ++column;
          return static_cast<int>(value - 'a');
      }
    });
    heightmap.add_row(row);
  }

  auto fewest_steps = get_fewest_steps<reverse>(heightmap);
  std::cout << filename << " -> " << fewest_steps << std::endl;
  return fewest_steps;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(31, solve_case<false>("day12.example"));
  AOC_EXPECT_RESULT(504, solve_case<false>("day12.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(29, solve_case<true>("day12.example"));
  AOC_EXPECT_RESULT(500, solve_case<true>("day12.input"));
  AOC_RETURN_CHECK_RESULT();
}
