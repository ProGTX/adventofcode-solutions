// https://adventofcode.com/2023/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <print>

using island_t = aoc::char_grid<>;

fn parse(const String& filename) -> island_t {
  return aoc::read_char_grid(filename);
}

fn solve_case1(island_t const& island_map) -> u32 {
  let start = point{1, 0};
  let end = point{static_cast<i32>(island_map.num_columns()) - 2,
                  static_cast<i32>(island_map.num_rows()) - 1};

  let distances = aoc::longest_distances(
      start, [&](const point& pos) { return pos == end; },
      [&](const point& pos) {
        auto neighbors =
            aoc::static_vector<aoc::dijkstra_neighbor_t<point>, 4>{};

        for (let diff : aoc::basic_neighbor_diffs) {
          let neighbor = pos + diff;
          if (!island_map.in_bounds(neighbor.y, neighbor.x)) {
            continue;
          }
          // Slopes may only be entered in their direction,
          // which prevents cycles and keeps the graph a DAG.
          switch (island_map.at(neighbor.y, neighbor.x)) {
            case '#':
              continue;
            case '>':
              if (diff != point{1, 0}) {
                continue;
              }
              break;
            case '<':
              if (diff != point{-1, 0}) {
                continue;
              }
              break;
            case '^':
              if (diff != point{0, -1}) {
                continue;
              }
              break;
            case 'v':
              if (diff != point{0, 1}) {
                continue;
              }
              break;
            default:
              break;
          }
          neighbors.emplace_back(neighbor, 1);
        }
        return neighbors;
      });

  return distances.at(end);
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(94, solve_case1(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(2326, solve_case1(input));

  aoc::return_incomplete();
  AOC_RETURN_CHECK_RESULT();
}
