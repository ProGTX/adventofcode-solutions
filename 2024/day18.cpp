// https://adventofcode.com/2024/day/18

#include "../common/common.h"

#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using memspace_t = aoc::char_grid<>;
constexpr const char corrupted = '#';
constexpr const char empty = '.';

template <int num_fallen>
constexpr memspace_t bytes_fall(memspace_t memspace,
                                std::span<const point> falling_bytes) {
  AOC_ASSERT(num_fallen <= falling_bytes.size(), "Too many bytes requested");
  for (const auto byte_pos : falling_bytes.subspan(0, num_fallen)) {
    memspace.modify(corrupted, byte_pos.y, byte_pos.x);
  }
  return memspace;
}

constexpr int shortest_path(const memspace_t& memspace) {
  const auto start_pos = point{0, 0};
  const auto end_pos =
      point(memspace.num_rows() - 1, memspace.num_columns() - 1);

  auto distances = aoc::shortest_distances_dijkstra(
      start_pos,
      [&](const point current) {
        return memspace.basic_neighbors(current) |
               std::views::filter([&](point neighbor) {
                 return memspace.at(neighbor.y, neighbor.x) != corrupted;
               }) |
               aoc::dijkstra_uniform_neighbors_view() |
               aoc::ranges::to<
                   aoc::static_vector<aoc::dijkstra_neighbor_t<point>, 4>>();
      },
      end_pos);

  return distances[end_pos];
}

template <point grid_size, int num_fallen>
int solve_case(const std::string& filename) {
  std::vector<point> falling_bytes;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    falling_bytes.push_back(aoc::split<point>(line, ','));
  }

  memspace_t memspace{aoc::views::repeat(empty, grid_size.y * grid_size.x) |
                          aoc::ranges::to<std::string>(),
                      grid_size.y, grid_size.x};

  int sum = 0;
  sum = shortest_path(bytes_fall<num_fallen>(memspace, falling_bytes));

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(22, (solve_case<{7, 7}, 12>("day18.example")));
  AOC_EXPECT_RESULT(234, (solve_case<{71, 71}, 1024>("day18.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, (solve_case<{7,7},12>("day18.example")));
  // AOC_EXPECT_RESULT(53515, (solve_case<{71,71},1024>("day18.input")));
  AOC_RETURN_CHECK_RESULT();
}
