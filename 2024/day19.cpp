// https://adventofcode.com/2024/day/19

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr bool is_design_possible(std::string_view design,
                                  std::span<const std::string> patterns) {
  bool end_found = false;
  aoc::shortest_distances_dijkstra(
      std::string{""},
      [&](const std::string& current) {
        auto neighbors = std::vector<aoc::dijkstra_neighbor_t<std::string>>{};
        for (const std::string& pattern : patterns) {
          auto neighbor = current + pattern;
          if (design.starts_with(neighbor)) {
            neighbors.emplace_back(std::move(neighbor),
                                   design.size() - neighbor.size() + 1);
          }
        }
        return neighbors;
      },
      [&](std::string_view current) {
        if (current.size() == design.size()) {
          end_found = true;
        }
        return end_found;
      });
  return end_found;
}

constexpr int check_all_designs(std::span<const std::string> designs,
                                std::span<const std::string> patterns) {
  return std::ranges::count_if(designs, [&](std::string_view design) {
    return is_design_possible(design, patterns);
  });
}

template <bool>
int solve_case(const std::string& filename) {
  std::vector<std::string> designs;
  std::vector<std::string> patterns;
  for (int row = 0; std::string line : aoc::views::read_lines(filename)) {
    if (row < 1) {
      patterns = aoc::split<std::vector<std::string>>(
          line, ',', aoc::trimmer<std::string>());
    } else {
      designs.push_back(std::move(line));
    }
    ++row;
  }

  int sum = 0;
  sum = check_all_designs(designs, patterns);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6, solve_case<false>("day19.example"));
  AOC_EXPECT_RESULT(317, solve_case<false>("day19.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day19.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day19.input"));
  AOC_RETURN_CHECK_RESULT();
}
