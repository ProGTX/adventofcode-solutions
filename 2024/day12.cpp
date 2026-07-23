// https://adventofcode.com/2024/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <span>
#include <unordered_set>

using garden_t = aoc::char_grid<>;
using region_t = std::unordered_set<usize>;
// Pad the area with edges to remove the need for bounds checking
constexpr let padding = '#';

using side_t = aoc::closed_range<point>;

fn get_regions(garden_t const& garden) -> Vec<region_t> {
  let get_region = [&](usize start) {
    return aoc::flood_fill(start, [&](usize current) {
      let current_pos = garden.position(current);
      let current_value = garden.at(current_pos.y, current_pos.x);
      // current_pos/current_value must be captured by value:
      // the returned view is iterated after this lambda call returns,
      // so any capture by reference to these locals would dangle.
      return aoc::basic_neighbor_diffs |
             aoc::views::transform_filter(
                 [&, current_pos, current_value](point diff) -> Option<usize> {
                   let neighbor = current_pos + diff;
                   if (garden.at(neighbor.y, neighbor.x) != current_value) {
                     return None;
                   }
                   return garden.linear_index(neighbor.y, neighbor.x);
                 });
    });
  };
  auto regions = Vec<region_t>{};
  regions.push_back(get_region(garden.linear_index(1, 1)));
  for (usize linear_index : Range{0uz, garden.size()}) {
    if ((garden.at_index(linear_index) == padding) ||
        stdr::any_of(regions, [&](region_t const& region) {
          return region.contains(linear_index);
        })) {
      continue;
    }
    regions.push_back(get_region(linear_index));
  }
  return regions;
}

struct Input {
  garden_t garden;
  Vec<region_t> regions;
};

auto parse(String const& filename) -> Input {
  auto garden = aoc::read_char_grid(filename, {.padding = padding}).first;
  auto regions = get_regions(garden);
  return {std::move(garden), std::move(regions)};
}

fn perimeter(garden_t const& garden, region_t const& region) -> Vec<side_t> {
  auto occurrence = aoc::flat_map<side_t, u32>{};
  for (usize index : region) {
    // We need to convert a single plant into 4 sides of a square
    let current = garden.position(index);
    let corners = std::array{
        current,
        point{current.x + 1, current.y},
        point{current.x + 1, current.y + 1},
        point{current.x, current.y + 1},
        current, // Needs to be circular
    };
    for (let[a, b] : corners | stdv::pairwise) {
      // side_t's constructor orders begin/end for us
      occurrence[side_t{a, b}] += 1;
    }
  }
  return occurrence |
         stdv::filter([](let& entry) { return entry.second == 1; }) |
         stdv::transform([](let& entry) { return entry.first; }) |
         aoc::collect_vec<side_t>();
}

fn solve_case1(Input const& input) -> usize {
  return aoc::ranges::accumulate(
      input.regions | stdv::transform([&](region_t const& region) {
        return region.size() * perimeter(input.garden, region).size();
      }),
      usize{});
}

// Which side of a fence segment the region sits on,
// i.e. which of the segment's two flanking cells is part of `region`.
fn fence_direction(garden_t const& garden, region_t const& region,
                   side_t const& side) -> aoc::facing_t {
  if (side.begin.y == side.end.y) {
    let below = garden.linear_index(side.begin.y, side.begin.x);
    return region.contains(below) ? aoc::north : aoc::south;
  } else {
    let right = garden.linear_index(side.begin.y, side.begin.x);
    return region.contains(right) ? aoc::west : aoc::east;
  }
}

// Merges fence segments into sides: segments in the same direction,
// collinear (sharing a row for North/South, a column for East/West),
// and consecutive along that line, belong to the same straight side.
fn count_sides(garden_t const& garden, region_t const& region,
               std::span<const side_t> fence) -> usize {
  auto groups = aoc::flat_map<std::pair<aoc::facing_t, int>, Vec<int>>{};
  // Bucket each segment by its direction and the line it lies on,
  // recording its position along that line.
  for (side_t const& side : fence) {
    let direction = fence_direction(garden, region, side);
    int line;
    int position;
    if ((direction == aoc::north) || (direction == aoc::south)) {
      line = side.begin.y;
      position = side.begin.x;
    } else {
      AOC_ASSERT((direction == aoc::east) || (direction == aoc::west),
                 "perimeter fence segments are always axis-aligned");
      line = side.begin.x;
      position = side.begin.y;
    }
    groups[{direction, line}].push_back(position);
  }

  // Within each bucket, count runs of consecutive positions:
  // each gap starts a new side, so a side is one run.
  return aoc::ranges::accumulate(
      groups | stdv::values | stdv::transform([](Vec<int>& positions) {
        stdr::sort(positions);
        return 1 + stdr::count_if(positions | stdv::pairwise, [](auto pair) {
                 let[a, b] = pair;
                 return b != a + 1;
               });
      }),
      usize{});
}

fn solve_case2(Input const& input) -> usize {
  return aoc::ranges::accumulate(
      input.regions | stdv::transform([&](region_t const& region) {
        let fence = perimeter(input.garden, region);
        return region.size() * count_sides(input.garden, region, fence);
      }),
      usize{});
}

int main() {
  std::println("Part 1");
  let example = parse("day12.example");
  AOC_EXPECT_RESULT(140, solve_case1(example));
  let example2 = parse("day12.example2");
  AOC_EXPECT_RESULT(772, solve_case1(example2));
  let example3 = parse("day12.example3");
  AOC_EXPECT_RESULT(1930, solve_case1(example3));
  let example4 = parse("day12.example4");
  AOC_EXPECT_RESULT(692, solve_case1(example4));
  let example5 = parse("day12.example5");
  AOC_EXPECT_RESULT(1184, solve_case1(example5));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(1450422, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(80, solve_case2(example));
  AOC_EXPECT_RESULT(436, solve_case2(example2));
  AOC_EXPECT_RESULT(1206, solve_case2(example3));
  AOC_EXPECT_RESULT(236, solve_case2(example4));
  AOC_EXPECT_RESULT(368, solve_case2(example5));
  AOC_EXPECT_RESULT(906606, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
