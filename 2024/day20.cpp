// https://adventofcode.com/2024/day/20

#include "../common/common.h"

#include <array>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using racetrack_t = aoc::char_grid<>;
constexpr const char start = 'S';
constexpr const char end = 'E';
constexpr const char wall = '#';
constexpr const char empty = '.';

struct node_t {
  point pos;
  int cheat_length;

  constexpr bool operator==(const node_t&) const = default;
  constexpr auto operator<=>(const node_t&) const = default;
};
using predecessors_t = aoc::predecessor_map<node_t>;
using path_t = std::vector<node_t>;

template <bool full_path>
constexpr auto shortest_path(const racetrack_t& track, const point start_pos,
                             const point end_pos, const int max_cheat_length) {
  node_t end_node;
  predecessors_t predecessors;
  const auto distances = aoc::shortest_distances_dijkstra(
      node_t{start_pos, 0},
      [&](const node_t& node) {
        auto neighbors =
            aoc::static_vector<aoc::dijkstra_neighbor_t<node_t>, 4>{};
        for (auto neighbor_pos : track.basic_neighbors(node.pos)) {
          const auto neighbor = track.at(neighbor_pos.y, neighbor_pos.x);
          int cheat_length = node.cheat_length;
          if (neighbor == wall) {
            if (cheat_length < max_cheat_length) {
              neighbors.emplace_back(node_t{neighbor_pos, cheat_length + 1}, 1);
            }
          } else {
            if (cheat_length > 0) {
              cheat_length = std::max(cheat_length + 1, max_cheat_length);
            }
            neighbors.emplace_back(node_t{neighbor_pos, cheat_length}, 1);
          }
        }
        return neighbors;
      },
      [&](const node_t& node) {
        if (node.pos == end_pos) {
          end_node = node;
          return true;
        }
        return false;
      },
      &predecessors);
  if constexpr (!full_path) {
    return distances.at(end_node);
  } else {
    return aoc::get_path(predecessors, end_node);
  }
}

constexpr int count_cheats(const racetrack_t& track, const point start_pos,
                           const point end_pos) {
  const path_t path = shortest_path<true>(track, start_pos, end_pos, 0);
  aoc::flat_set<point> possible_cheats;
  for (const node_t& node : path) {
    for (auto neighbor_pos : track.basic_neighbors(node.pos)) {
      if ((neighbor_pos.x == 0) ||
          (neighbor_pos.x == (track.num_columns() - 1)) ||
          (neighbor_pos.y == 0) || (neighbor_pos.y == (track.num_rows() - 1))) {
        // Skip the edge of the track
        continue;
      }
      const auto neighbor = track.at(neighbor_pos.y, neighbor_pos.x);
      if (neighbor == wall) {
        possible_cheats.emplace(neighbor_pos);
      }
    }
  }
  int num_cheats = 0;
  // return possible_cheats.size();
  const auto time_limit = (track.row_length() < 20) ? 20 : 100;
  auto mutable_track = track;
  for (int cheat_id = 0; point cheat : possible_cheats) {
    mutable_track.modify(empty, cheat.y, cheat.x);
    // const auto cheat_path =
    const auto cheat_length =
        shortest_path<false>(mutable_track, start_pos, end_pos, 0);
    // const auto cheat_length = static_cast<int>(cheat_path.size());
    const auto time_diff = static_cast<int>(path.size()) - cheat_length;
    if (time_diff >= time_limit) {
      ++num_cheats;
    }
    mutable_track.modify(wall, cheat.y, cheat.x);
    ++cheat_id;
  }
  return num_cheats;
}

template <bool>
int solve_case(const std::string& filename) {
  auto [track, config] =
      aoc::read_char_grid(filename, aoc::char_grid_config_input{
                                        .padding = {},
                                        .start_char = start,
                                        .end_char = end,
                                    });

  int sum = 0;
  sum = count_cheats(track, *config.start_pos, *config.end_pos);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(5, solve_case<false>("day20.example"));
  AOC_EXPECT_RESULT(1459, solve_case<false>("day20.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day20.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day20.input"));
  AOC_RETURN_CHECK_RESULT();
}
