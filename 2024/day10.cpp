// https://adventofcode.com/2024/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Highest value on the map is 9, so 11 can never be reached
constexpr const char trailhead_char = '0';
constexpr const int edge = 11;
constexpr const int trail_end = 9;
using top_map_t = aoc::grid<int>;

constexpr top_map_t test_map() {
  return {std::vector{11, 11, 11, 11, 11, 11, //
                      11, 0,  1,  2,  3,  11, //
                      11, 1,  2,  3,  4,  11, //
                      11, 8,  7,  6,  5,  11, //
                      11, 9,  8,  7,  6,  11, //
                      11, 11, 11, 11, 11, 11},
          6, 6};
}

constexpr int get_score(const top_map_t& top_map, point trailhead) {
  aoc::flat_set<point> end_points;
  aoc::flat_set<point> unvisited;
  unvisited.insert(trailhead);
  while (!unvisited.empty()) {
    // We take from the back because that's more efficient for a flat_set
    auto current_it = unvisited.end() - 1;
    auto current = *current_it;
    unvisited.erase(current_it);
    const auto current_height = top_map.at(current.y, current.x);
    if (current_height == (trail_end - 1)) {
      // Neighbors would finish search
      std::ranges::copy(
          aoc::basic_neighbor_diffs | std::views::transform([&](point diff) {
            return current + diff;
          }) | std::views::filter([&](point neighbor) {
            return top_map.at(neighbor.y, neighbor.x) == trail_end;
          }),
          aoc::inserter_it(end_points));
    } else {
      // Neighbors might progress search
      std::ranges::copy(
          aoc::basic_neighbor_diffs | std::views::transform([&](point diff) {
            return current + diff;
          }) | std::views::filter([&](point neighbor) {
            return top_map.at(neighbor.y, neighbor.x) == current_height + 1;
          }),
          aoc::inserter_it(unvisited));
    }
  }
  return end_points.size();
}

constexpr int score_trailheads(const top_map_t& top_map,
                               std::span<const point> trailheads) {
  return aoc::ranges::accumulate(
      trailheads | std::views::transform([&](point trailhead) {
        return get_score(top_map, trailhead);
      }),
      0);
}

template <bool>
int solve_case(const std::string& filename) {
  // Pad the map with edges so that we don't have to do bounds checking
  top_map_t top_map;
  std::vector<point> trailheads;

  for (int row_id = 1;
       std::string_view line : aoc::views::read_lines(filename)) {
    if (top_map.empty()) {
      top_map.add_row(std::views::repeat(edge, line.size() + 2));
    }
    std::vector<int> row;
    row.push_back(edge);
    std::ranges::copy(
        line | std::views::enumerate |
            std::views::transform([&](auto&& current) {
              auto [col_id, c] = current;
              if (c == trailhead_char) {
                trailheads.push_back({static_cast<int>(col_id) + 1, row_id});
              }
              return aoc::to_number<int>(c);
            }),
        std::back_inserter(row));
    row.push_back(edge);
    top_map.add_row(std::move(row));
    ++row_id;
  }
  top_map.add_row(std::views::repeat(edge, top_map.row_length()));

  int sum = 0;
  sum = score_trailheads(top_map, trailheads);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1, solve_case<false>("day10.example"));
  AOC_EXPECT_RESULT(36, solve_case<false>("day10.example2"));
  AOC_EXPECT_RESULT(652, solve_case<false>("day10.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day10.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
