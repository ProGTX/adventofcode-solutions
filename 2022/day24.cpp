// https://adventofcode.com/2022/day/24

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <numeric>
#include <print>
#include <ranges>

// Each cell stores a bitmask of which blizzard directions are present.
// Bit index = static_cast<int>(facing_t): east=0, south=1, west=2, north=3.
using blizzard_grid_t = aoc::grid<u8>;
using blizzard_cache_t = Vec<blizzard_grid_t>;

struct Input {
  point dimensions;
  blizzard_cache_t blizzard_cache;
};

fn move_blizzards(blizzard_grid_t const& blizzards, const point inner)
    -> blizzard_grid_t {
  auto result =
      blizzard_grid_t(u8{0}, blizzards.num_rows(), blizzards.num_columns());
  let rem_euclid = [](int val, int n) { return ((val % n) + n) % n; };
  for (int row = 0; row < static_cast<int>(blizzards.num_rows()); ++row) {
    for (int col = 0; col < static_cast<int>(blizzards.num_columns()); ++col) {
      let cell = blizzards.at(row, col);
      if (cell == 0) {
        continue;
      }
      for (let dir : aoc::basic_sky_directions) {
        let bit = static_cast<u8>(1 << static_cast<int>(dir));
        if ((cell & bit) != 0) {
          let diff = aoc::get_diff<int>(dir);
          let new_col = rem_euclid(col - 1 + diff.x, inner.x) + 1;
          let new_row = rem_euclid(row - 1 + diff.y, inner.y) + 1;
          result.at(new_row, new_col) |= bit;
        }
      }
    }
  }
  return result;
}

// Pre-compute all blizzard snapshots for one full period.
// Blizzards are periodic with period = lcm(inner_width, inner_height).
fn precompute_blizzards(blizzard_grid_t initial, const point dimensions)
    -> blizzard_cache_t {
  let inner = dimensions - point{2, 2};
  let period = std::lcm(inner.x, inner.y);
  auto cache = blizzard_cache_t{};
  cache.reserve(static_cast<usize>(period));
  cache.push_back(std::move(initial));
  for (int i = 1; i < period; ++i) {
    cache.push_back(move_blizzards(cache.back(), inner));
  }
  return cache;
}

fn parse(String const& filename) -> Input {
  let char_grid = aoc::read_char_grid(filename);
  let num_rows = static_cast<int>(char_grid.num_rows());
  let num_columns = static_cast<int>(char_grid.num_columns());
  let dims = point{num_columns, num_rows};
  auto blizzards =
      blizzard_grid_t(u8{0}, char_grid.num_rows(), char_grid.num_columns());
  for (int row = 1; row < num_rows - 1; ++row) {
    for (int col = 1; col < num_columns - 1; ++col) {
      let c = char_grid.at(row, col);
      if (c == '>' || c == '<' || c == '^' || c == 'v') {
        let facing = aoc::to_facing(c);
        blizzards.at(row, col) |=
            static_cast<u8>(1 << static_cast<int>(facing));
      }
    }
  }
  return {dims, precompute_blizzards(std::move(blizzards), dims)};
}

// State is just position + time mod period - no grid clone needed.
struct search_state_t {
  point pos;
  int time;

  constexpr bool operator==(const search_state_t&) const = default;
  constexpr auto operator<=>(const search_state_t&) const = default;
};

// Returns {end_time, distance}.
// end_time is the time mod period at arrival, used to chain into the next leg.
fn find_distance(Input const& input, const point start_pos, const point end_pos,
                 int const start_time) -> std::pair<int, int> {
  let & [ dimensions, blizzard_cache ] = input;
  let period = static_cast<int>(blizzard_cache.size());
  let on_blizzard = [&](blizzard_grid_t const& grid, const point pos) {
    return grid.at(pos.y, pos.x) != 0;
  };
  let distances = aoc::shortest_distances_astar(
      search_state_t{start_pos, start_time},
      [&](search_state_t const& current) { return current.pos == end_pos; },
      [&](search_state_t const& current) {
        let next_time = (current.time + 1) % period;
        let& new_blizzards = blizzard_cache[static_cast<usize>(next_time)];
        auto neighbors = Vec<aoc::dijkstra_neighbor_t<search_state_t>>{};
        for (let dir : aoc::basic_sky_directions) {
          let new_pos = current.pos + aoc::get_diff<int>(dir);
          if ((new_pos != start_pos) &&
              (new_pos != end_pos) &&
              ((new_pos.x <= 0) ||
               (new_pos.y <= 0) ||
               (new_pos.x == dimensions.x - 1) ||
               (new_pos.y == dimensions.y - 1))) {
            continue;
          }
          if (!new_blizzards.in_bounds(new_pos.y, new_pos.x) ||
              on_blizzard(new_blizzards, new_pos)) {
            continue;
          }
          neighbors.push_back({.node = {new_pos, next_time}, .distance = 1});
        }
        if (!on_blizzard(new_blizzards, current.pos)) {
          neighbors.push_back(
              {.node = {current.pos, next_time}, .distance = 1});
        }
        return neighbors;
      },
      [&](search_state_t const& current) {
        return static_cast<int>(distance_manhattan(current.pos, end_pos));
      });
  let it = stdr::find_if(
      distances, [&](let& entry) { return entry.first.pos == end_pos; });
  return {it->first.time, it->second};
}

fn solve_case1(Input const& input) -> int {
  let & [ dimensions, _ ] = input;
  let start_pos = point{1, 0};
  let end_pos = point{dimensions.x - 2, dimensions.y - 1};
  return find_distance(input, start_pos, end_pos, 0).second;
}

fn solve_case2(Input const& input) -> int {
  let & [ dimensions, _ ] = input;
  let start_pos = point{1, 0};
  let end_pos = point{dimensions.x - 2, dimensions.y - 1};
  let[t1, d1] = find_distance(input, start_pos, end_pos, 0);
  let[t2, d2] = find_distance(input, end_pos, start_pos, t1);
  let[_, d3] = find_distance(input, start_pos, end_pos, t2);
  return d1 + d2 + d3;
}

int main() {
  std::println("Part 1");
  let example = parse("day24.example");
  AOC_EXPECT_RESULT(10, solve_case1(example));
  let example2 = parse("day24.example2");
  AOC_EXPECT_RESULT(18, solve_case1(example2));
  let input = parse("day24.input");
  AOC_EXPECT_RESULT(228, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(30, solve_case2(example));
  AOC_EXPECT_RESULT(54, solve_case2(example2));
  AOC_EXPECT_RESULT(723, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
