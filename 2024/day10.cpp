// https://adventofcode.com/2024/day/10

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>

// Highest value on the map is 9, so 11 can never be reached
constexpr let trailhead_char = '0';
constexpr let edge_char = '.';
constexpr let edge = u8{11};
constexpr let trail_end = u8{9};
using top_map_t = aoc::grid<u8>;

struct Input {
  top_map_t top_map;
  Vec<point> trailheads;
};

auto parse(String const& filename) -> Input {
  let char_map = aoc::read_char_grid(filename, {.padding = edge_char}).first;

  auto trailheads =
      aoc::views::indices_of(char_map) |
      stdv::filter(
          [&](usize i) { return char_map.at_index(i) == trailhead_char; }) |
      stdv::transform([&](usize i) { return char_map.position(i); }) |
      aoc::collect_vec<point>();

  auto heights = //
      char_map |
      stdv::transform([](char c) {
        return (c == edge_char) ? edge : aoc::to_number<u8>(c);
      }) |
      aoc::collect_vec<u8>();

  return {top_map_t{std::move(heights), char_map.num_rows(),
                    char_map.num_columns()},
          std::move(trailheads)};
}
fn test_map() -> top_map_t {
  return {std::vector{11, 11, 11, 11, 11, 11, //
                      11, 0,  1,  2,  3,  11, //
                      11, 1,  2,  3,  4,  11, //
                      11, 8,  7,  6,  5,  11, //
                      11, 9,  8,  7,  6,  11, //
                      11, 11, 11, 11, 11, 11} |
              aoc::collect_vec<u8>(),
          6, 6};
}

fn get_score(top_map_t const& top_map, point trailhead) -> usize {
  // Height strictly increases by 1 along every edge,
  // so the graph is a DAG and every path to a given cell has the same length -
  // reachability and "shortest distance" coincide,
  // so we can use Dijkstra to explore the whole trail from the trailhead
  let distances =
      aoc::shortest_distances_dijkstra(trailhead, [&](point current) {
        let current_height = top_map.at(current.y, current.x);
        return aoc::basic_neighbor_diffs |
               stdv::transform(
                   [current](point diff) { return current + diff; }) |
               stdv::filter([&top_map, current_height](point neighbor) {
                 return top_map.at(neighbor.y, neighbor.x) ==
                        current_height + 1;
               }) |
               aoc::dijkstra_uniform_neighbors_view();
      });

  return stdr::count_if(distances | stdv::keys, [&](point const& pos) {
    return top_map.at(pos.y, pos.x) == trail_end;
  });
}
static_assert(1 == get_score(test_map(), {1, 1}));

fn solve_case1(Input const& input) -> usize {
  return aoc::ranges::accumulate(
      input.trailheads | stdv::transform([&](point trailhead) {
        return get_score(input.top_map, trailhead);
      }),
      usize{});
}

struct SearchState {
  point pos;
  Vec<point> trail;

  constexpr bool operator==(SearchState const&) const = default;
};
template <>
struct std::hash<SearchState> {
  size_t operator()(SearchState const& state) const {
    auto combine = aoc::hash_combine{};
    combine(std::hash<point>{}(state.pos));
    for (point const& p : state.trail) {
      combine(std::hash<point>{}(p));
    }
    return combine.seed;
  }
};

fn get_rating(top_map_t const& top_map, point trailhead) -> usize {
  let start = SearchState{
      .pos = trailhead,
      .trail = {},
  };
  let num_trails = aoc::dfs_uniform(
      start,
      [&](SearchState const& state) {
        return top_map.at(state.pos.y, state.pos.x) == trail_end;
      },
      [&](SearchState const& state) {
        let current_height = top_map.at(state.pos.y, state.pos.x);
        return aoc::basic_neighbor_diffs |
               stdv::transform([&](point diff) { return state.pos + diff; }) |
               stdv::filter([&top_map, current_height](point neighbor) {
                 return top_map.at(neighbor.y, neighbor.x) ==
                        current_height + 1;
               }) |
               stdv::transform([&](point neighbor) {
                 auto new_state = state;
                 new_state.pos = neighbor;
                 new_state.trail.push_back(state.pos);
                 return new_state;
               });
      });
  return num_trails.at(start);
}

fn solve_case2(Input const& input) -> usize {
  return aoc::ranges::accumulate(
      input.trailheads | stdv::transform([&](point trailhead) {
        return get_rating(input.top_map, trailhead);
      }),
      usize{});
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  AOC_EXPECT_RESULT(1, solve_case1(example));
  let example2 = parse("day10.example2");
  AOC_EXPECT_RESULT(36, solve_case1(example2));
  let input = parse("day10.input");
  AOC_EXPECT_RESULT(652, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(81, solve_case2(example2));
  AOC_EXPECT_RESULT(1432, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
