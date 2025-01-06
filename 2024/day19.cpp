// https://adventofcode.com/2024/day/19

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

template <bool all_options>
constexpr int_t num_possible_designs(std::string_view design,
                                     std::span<const std::string> patterns);

template <>
constexpr int_t num_possible_designs<false>(
    std::string_view design, std::span<const std::string> patterns) {
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

constexpr int_t count_designs(aoc::flat_map<std::string, int_t>& cache,
                              const std::string& design,
                              std::span<const std::string> patterns) {
  if (design.empty()) {
    return 1;
  }
  auto it = cache.find(design);
  if (it != std::end(cache)) {
    return it->second;
  }
  int_t sum = 0;
  for (std::string_view p : patterns) {
    if (design.starts_with(p)) {
      sum += count_designs(cache, design.substr(p.size()), patterns);
    }
  }
  cache[design] = sum;
  return sum;
}

template <>
constexpr int_t num_possible_designs<true>(
    std::string_view design, std::span<const std::string> patterns) {
  aoc::flat_map<std::string, int_t> cache;
  return count_designs(cache, std::string{design}, patterns);
}

template <bool all_options>
constexpr int_t check_all_designs(std::span<const std::string> designs,
                                  std::span<const std::string> patterns) {
  return aoc::ranges::accumulate(
      designs | std::views::transform([&](std::string_view design) {
        return num_possible_designs<all_options>(design, patterns);
      }),
      int_t{0});
}

template <bool all_options>
int_t solve_case(const std::string& filename) {
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

  int_t sum = 0;
  sum = check_all_designs<all_options>(designs, patterns);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6, solve_case<false>("day19.example"));
  AOC_EXPECT_RESULT(317, solve_case<false>("day19.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(16, solve_case<true>("day19.example"));
  AOC_EXPECT_RESULT(883443544805484, solve_case<true>("day19.input"));
  AOC_RETURN_CHECK_RESULT();
}
