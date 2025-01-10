// https://adventofcode.com/2024/day/20

#include "../common/common.h"

#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using racetrack_t = aoc::char_grid<>;
constexpr const char start = 'S';
constexpr const char end = 'E';
constexpr const char wall = '#';

struct node_t {
  point pos{};
  int length{0};
  bool cheated{false};

  constexpr bool operator==(const node_t& other) const {
    return (pos == other.pos);
  }
  constexpr auto operator<=>(const node_t& other) const {
    return (pos <=> other.pos);
  }
};
using predecessors_t = aoc::predecessor_map<node_t>;
using path_t = std::vector<node_t>;

template <int max_length, bool wall_allowed>
constexpr auto shortest_path(const racetrack_t& track, const point start_pos,
                             const point end_pos) {
  predecessors_t predecessors;
  auto predecessors_ptr = &predecessors;
  if constexpr (wall_allowed) {
    predecessors_ptr = nullptr;
  }
  auto end_nodes =
      std::conditional_t<wall_allowed, aoc::flat_set<node_t>, node_t>{};
  aoc::shortest_distances_dijkstra(
      node_t{start_pos, 0, false},
      [&](const node_t& node) {
        auto neighbors =
            aoc::static_vector<aoc::dijkstra_neighbor_t<node_t>, 4>{};
        if ((node.length >= max_length) || (node.pos == end_pos)) {
          return neighbors;
        }
        for (auto neighbor_pos : track.basic_neighbors(node.pos)) {
          const auto neighbor = track.at(neighbor_pos.y, neighbor_pos.x);
          if ((wall_allowed && (neighbor == wall)) || (neighbor != wall)) {
            bool cheated = node.cheated;
            if constexpr (wall_allowed) {
              if (neighbor == wall) {
                if (node.length == (max_length - 1)) {
                  // Can't end in a wall
                  continue;
                }
                cheated = true;
              }
            }
            neighbors.emplace_back(
                node_t{neighbor_pos, node.length + 1, cheated}, 1);
          }
        }
        return neighbors;
      },
      [&](const node_t& node) {
        if constexpr (!wall_allowed) {
          if (node.pos == end_pos) {
            end_nodes = node;
            return true;
          }
        } else {
          if (node.cheated && (node.length > 1) &&
              (track.at(node.pos.y, node.pos.x) != wall)) {
            end_nodes.emplace(node);
          }
        }
        return false;
      },
      predecessors_ptr);
  if constexpr (!wall_allowed) {
    return aoc::get_path(predecessors, end_nodes);
  } else {
    return end_nodes;
  }
}

struct segment_t {
  point begin;
  point end;

  constexpr bool operator==(const segment_t&) const = default;
  constexpr auto operator<=>(const segment_t&) const = default;
};

using cache_t = aoc::flat_map<segment_t, int>;
constexpr const int max_normal_length = (1 << 20);

constexpr int normal_shortest_length(cache_t& cache, const racetrack_t& track,
                                     const point start_pos,
                                     const point end_pos) {
  auto segment = segment_t{start_pos, end_pos};
  auto it = cache.find(segment);
  if (it != std::end(cache)) {
    return it->second;
  }
  const auto length =
      shortest_path<max_normal_length, false>(track, start_pos, end_pos).size();
  cache[segment] = length;
  return length;
}

template <int max_cheat_length>
constexpr int count_cheats(const racetrack_t& track, const point start_pos,
                           const point end_pos) {
  const path_t path =
      shortest_path<max_normal_length, false>(track, start_pos, end_pos);
  const int standard_length = path.size();
  aoc::flat_set<segment_t> cheats;
  cache_t cache;
  const auto time_limit = (track.row_length() < 20) ? 50 : 100;
  for (int path_length = 0; const node_t& node : path | std::views::reverse) {
    cache[segment_t{node.pos, end_pos}] = standard_length - path_length;
    ++path_length;
  }
  for (int path_length = 0; const node_t& node : path | std::views::reverse) {
    // end_pos doesn't matter here
    const auto cheat_points =
        shortest_path<max_cheat_length, true>(track, node.pos, {});
    for (const node_t& cheat_end_node : cheat_points) {
      const auto cheat_length =
          path_length + distance_manhattan(node.pos, cheat_end_node.pos) +
          normal_shortest_length(cache, track, cheat_end_node.pos, end_pos);
      const auto time_diff = standard_length - cheat_length;
      if (time_diff >= time_limit) {
        cheats.emplace(node.pos, cheat_end_node.pos);
      }
    }
    ++path_length;
  }
  return cheats.size();
}

template <int max_cheat_length>
int solve_case(const std::string& filename) {
  auto [track, config] = aoc::read_char_grid(
      filename, {.padding = {}, .start_char = start, .end_char = end});

  int sum = 0;
  sum =
      count_cheats<max_cheat_length>(track, *config.start_pos, *config.end_pos);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1, solve_case<2>("day20.example"));
  AOC_EXPECT_RESULT(1459, solve_case<2>("day20.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(285, solve_case<20>("day20.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<20>("day20.input"));
  AOC_RETURN_CHECK_RESULT();
}
