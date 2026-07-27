// https://adventofcode.com/2024/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

using racetrack_t = aoc::char_grid<>;
constexpr let start = 'S';
constexpr let end = 'E';
constexpr let wall = '#';
constexpr let empty = '.';

struct Input {
  racetrack_t track;
  point start_pos;
  point end_pos;
};

auto parse(String const& filename) -> Input {
  auto [track, config] = aoc::read_char_grid(
      filename, {.padding = {}, .start_char = start, .end_char = end});
  let start_pos = *config.start_pos;
  let end_pos = *config.end_pos;
  track.modify(empty, start_pos.y, start_pos.x);
  track.modify(empty, end_pos.y, end_pos.x);
  return {std::move(track), start_pos, end_pos};
}

template <i32 target_ps_saved, i32 max_cheat_length>
fn solve_case(Input const& input) -> u32 {
  // Distance from start_pos to every track cell reachable from it
  // The track is a single, branch-free corridor, so this covers the whole track
  auto predecessors = aoc::predecessor_map<point>{};
  let distances = aoc::shortest_distances_dijkstra(
      input.start_pos, [&](point current) { return current == input.end_pos; },
      [&](point current) {
        return input.track.basic_neighbor_positions(current) |
               stdv::filter([&](point neighbor) {
                 return input.track.at(neighbor.y, neighbor.x) != wall;
               }) |
               aoc::dijkstra_uniform_neighbors_view();
      },
      &predecessors);
  // In start-to-end order, so a cheat's end always comes after its start
  auto track_cells = aoc::get_path(predecessors, input.end_pos);
  track_cells.push_back(input.start_pos);
  stdr::reverse(track_cells);

  auto count = u32{};
  for (let i : aoc::views::indices_of(track_cells)) {
    let cheat_start = track_cells[i];
    for (let j : Range{i + 1, track_cells.size()}) {
      let cheat_end = track_cells[j];
      let cheat_length = distance_manhattan(cheat_start, cheat_end);
      if ((cheat_length < 2) || (cheat_length > max_cheat_length)) {
        continue;
      }
      let saved =
          distances.at(cheat_end) - distances.at(cheat_start) - cheat_length;
      if (saved >= target_ps_saved) {
        ++count;
      }
    }
  }
  return count;
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(5, (solve_case<20, 2>(example)));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(1459, (solve_case<100, 2>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(285, (solve_case<50, 20>(example)));
  AOC_EXPECT_RESULT(1016066, (solve_case<100, 20>(input)));

  AOC_RETURN_CHECK_RESULT();
}
