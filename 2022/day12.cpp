// https://adventofcode.com/2022/day/12

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

using heightmap_t = grid<std::vector<int>>;
using neighbors_t = std::set<point, decltype(&point::distance_squared)>;
using path_t = std::vector<point>;
using possible_paths_t = std::vector<path_t>;

void get_path_recursive(const heightmap_t& heightmap,
                        possible_paths_t& possible_paths) {}

path_t get_path(const heightmap_t& heightmap) {
  possible_paths_t possible_paths;
  // Don't expect that many paths to be available
  possible_paths.reserve(10);

  get_path_recursive(heightmap, possible_paths);

  std::ranges::sort(possible_paths, [](const path_t& lhs, const path_t& rhs) {
    return (lhs.size() < rhs.size());
  });

  return possible_paths[0];
}

template <bool>
void solve_case(const std::string& filename) {
  int score = 0;

  using row_t = typename heightmap_t::row_t;
  heightmap_t heightmap;
  row_t row;

  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    row.clear();
    row.reserve(line.size());
    std::ranges::transform(line, std::back_inserter(row), [](char value) {
      switch (value) {
        case 'S':
          return 0;
        case 'E':
          return static_cast<int>('z' - 'a');
        default:
          return static_cast<int>(value - '0');
      }
    });
    heightmap.add_row(row);
  });

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day12.example");
  solve_case<false>("day12.input");
  // std::cout << "Part 2" << std::endl;
  solve_case<true>("day12.example");
  solve_case<true>("day12.input");
}
