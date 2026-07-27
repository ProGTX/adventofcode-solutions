// https://adventofcode.com/2024/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>
#include <span>

using racetrack_t = aoc::char_grid<>;
constexpr let start = 'S';
constexpr let end = 'E';
constexpr let wall = '#';

struct Input {
  racetrack_t track;
  point start_pos;
  point end_pos;
};

auto parse(String const& filename) -> Input {
  auto [track, config] = aoc::read_char_grid(
      filename, {.padding = {}, .start_char = start, .end_char = end});
  return {std::move(track), *config.start_pos, *config.end_pos};
}

struct node_t {
  point pos{};
  i32 length{0};
  bool cheated{false};

  constexpr bool operator==(node_t const& other) const {
    return (pos == other.pos);
  }
  constexpr auto operator<=>(node_t const& other) const {
    return (pos <=> other.pos);
  }
};
using predecessors_t = aoc::predecessor_map<node_t>;
using path_t = Vec<node_t>;

template <int max_length, bool wall_allowed>
fn shortest_path(racetrack_t const& track, point const start_pos,
                 point const end_pos) {
  auto predecessors = predecessors_t{};
  auto predecessors_ptr = &predecessors;
  if constexpr (wall_allowed) {
    predecessors_ptr = nullptr;
  }
  auto end_nodes =
      std::conditional_t<wall_allowed, aoc::flat_set<node_t>, node_t>{};
  aoc::shortest_distances_dijkstra(
      node_t{start_pos, 0, false},
      [&](node_t const& node) {
        if constexpr (!wall_allowed) {
          if (node.pos == end_pos) {
            end_nodes = node;
            return true;
          }
        } else {
          if (node.cheated &&
              (node.length > 1) &&
              (track.at(node.pos.y, node.pos.x) != wall)) {
            end_nodes.emplace(node);
          }
        }
        return false;
      },
      [&](node_t const& node) {
        auto neighbors =
            aoc::static_vector<aoc::dijkstra_neighbor_t<node_t>, 4>{};
        if ((node.length >= max_length) || (node.pos == end_pos)) {
          return neighbors;
        }
        for (let neighbor_pos : track.basic_neighbor_positions(node.pos)) {
          let neighbor = track.at(neighbor_pos.y, neighbor_pos.x);
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
      predecessors_ptr);
  if constexpr (!wall_allowed) {
    return aoc::get_path(predecessors, end_nodes);
  } else {
    return end_nodes;
  }
}

using segment_t = aoc::closed_range<point>;

using cache_t = aoc::flat_map<segment_t, i32>;
constexpr let max_normal_length = (1 << 20);

fn normal_shortest_length(cache_t& cache, racetrack_t const& track,
                          point const start_pos, point const end_pos) -> i32 {
  let segment = segment_t{start_pos, end_pos};
  let it = cache.find(segment);
  if (it != std::end(cache)) {
    return it->second;
  }
  let length = static_cast<i32>(
      shortest_path<max_normal_length, false>(track, start_pos, end_pos)
          .size());
  cache[segment] = length;
  return length;
}

template <int max_cheat_length>
fn count_cheats(racetrack_t const& track, point const start_pos,
                point const end_pos) -> i32 {
  let path = shortest_path<max_normal_length, false>(track, start_pos, end_pos);
  let standard_length = static_cast<i32>(path.size());
  auto cheats = aoc::flat_set<segment_t>{};
  auto cache = cache_t{};
  let time_limit = (track.row_length() < 20) ? 50 : 100;
  for (i32 path_length = 0; node_t const& node : path | stdv::reverse) {
    cache[segment_t{node.pos, end_pos}] = standard_length - path_length;
    ++path_length;
  }
  for (i32 path_length = 0; node_t const& node : path | stdv::reverse) {
    // end_pos doesn't matter here
    let cheat_points =
        shortest_path<max_cheat_length, true>(track, node.pos, {});
    for (node_t const& cheat_end_node : cheat_points) {
      let cheat_length =
          path_length +
          distance_manhattan(node.pos, cheat_end_node.pos) +
          normal_shortest_length(cache, track, cheat_end_node.pos, end_pos);
      let time_diff = standard_length - cheat_length;
      if (time_diff >= time_limit) {
        cheats.emplace(node.pos, cheat_end_node.pos);
      }
    }
    ++path_length;
  }
  return static_cast<i32>(cheats.size());
}

template <int max_cheat_length>
fn solve_case(Input const& input) -> i32 {
  return count_cheats<max_cheat_length>(input.track, input.start_pos,
                                        input.end_pos);
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(1, solve_case<2>(example));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(1459, solve_case<2>(input));

  std::println("Part 2");
  aoc::return_incomplete();
  // AOC_EXPECT_RESULT(14, solve_case<20>(example));
  // AOC_EXPECT_RESULT(1337, solve_case<20>(input));

  AOC_RETURN_CHECK_RESULT();
}
