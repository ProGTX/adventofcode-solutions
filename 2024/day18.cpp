// https://adventofcode.com/2024/day/18

#include "../common/common.h"

#include <array>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
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

constexpr std::optional<int> shortest_path(const memspace_t& memspace) {
  const auto start_pos = point{0, 0};
  const auto end_pos =
      point(memspace.num_rows() - 1, memspace.num_columns() - 1);

  const auto distances = aoc::shortest_distances_dijkstra(
      start_pos, end_pos, [&](const point current) {
        return memspace.basic_neighbor_positions(current) |
               stdv::filter([&](point neighbor) {
                 return memspace.at(neighbor.y, neighbor.x) != corrupted;
               }) |
               aoc::dijkstra_uniform_neighbors_view();
      });

  auto it = distances.find(end_pos);
  if (it == std::end(distances)) {
    return {};
  }
  return it->second;
}

constexpr memspace_t bytes_fall_dyn(memspace_t memspace,
                                    std::span<const point> falling_bytes,
                                    int count) {
  for (const auto byte_pos : falling_bytes.subspan(0, count)) {
    memspace.modify(corrupted, byte_pos.y, byte_pos.x);
  }
  return memspace;
}

template <int num_fallen>
constexpr point first_problematic_byte(const memspace_t& memspace,
                                       std::span<const point> falling_bytes) {
  // Adding corrupted cells can only ever block a path, never reopen one,
  // so whether a path exists is monotonic in the number of fallen bytes
  // That lets us binary search for the first blocking byte
  // instead of testing every byte one at a time
  const auto has_path_after = [&](int count) {
    return shortest_path(bytes_fall_dyn(memspace, falling_bytes, count))
        .has_value();
  };

  // We know from part 1 that num_fallen bytes don't cause a problem yet
  int low = num_fallen;
  int high = static_cast<int>(falling_bytes.size());
  if (has_path_after(high)) {
    return {};
  }
  while (low + 1 < high) {
    int mid = low + (high - low) / 2;
    if (has_path_after(mid)) {
      low = mid;
    } else {
      high = mid;
    }
  }

  return falling_bytes[high - 1];
}

template <point grid_size, int num_fallen, bool check_closed>
auto solve_case(const std::string& filename) {
  std::vector<point> falling_bytes;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    falling_bytes.push_back(aoc::split<point>(line, ','));
  }

  memspace_t memspace{
      stdv::repeat(empty, grid_size.y * grid_size.x) | aoc::collect_string(),
      grid_size.y, grid_size.x};

  auto result = std::conditional_t<check_closed, point, int>{};
  if constexpr (!check_closed) {
    result = *shortest_path(bytes_fall<num_fallen>(memspace, falling_bytes));
  } else {
    result = first_problematic_byte<num_fallen>(memspace, falling_bytes);
  }
  return result;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(22, (solve_case<{7, 7}, 12, false>("day18.example")));
  AOC_EXPECT_RESULT(234, (solve_case<{71, 71}, 1024, false>("day18.input")));
  std::println("Part 2");
  AOC_EXPECT_RESULT((point{6, 1}),
                    (solve_case<{7, 7}, 12, true>("day18.example")));
  AOC_EXPECT_RESULT((point{58, 19}),
                    (solve_case<{71, 71}, 1024, true>("day18.input")));
  AOC_RETURN_CHECK_RESULT();
}
