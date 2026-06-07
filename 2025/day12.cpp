// https://adventofcode.com/2025/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

struct RegionInfo {
  usize width;
  usize height;
  Vec<usize> counts;
};

using Input = std::pair<Vec<usize>, Vec<RegionInfo>>;

auto parse(String const& filename) -> Input {
  auto shape_areas = Vec<usize>{};
  auto regions = Vec<RegionInfo>{};
  auto current_shape_idx = usize{0};

  for (str line : aoc::views::read_lines(filename)) {
    if (line.ends_with(':')) {
      let[idx_str, rest] = aoc::split_once(line, ':');
      current_shape_idx = aoc::to_number<usize>(idx_str);
      shape_areas.resize(current_shape_idx + 1, 0);
      shape_areas[current_shape_idx] = 0;
    } else if (line.contains('#')) {
      shape_areas[current_shape_idx] +=
          static_cast<usize>(stdr::count(line, '#'));
    } else if (line.contains('x')) {
      let[dims, counts_str] = aoc::split_once(line, ':');
      let[width, height] = aoc::split_once<usize>(dims, 'x');
      let counts = aoc::split_sstream<usize>(counts_str);
      regions.push_back(RegionInfo{width, height, counts});
    }
  }

  return {std::move(shape_areas), std::move(regions)};
}

fn can_fit_into_region(Vec<usize> const& shape_areas, RegionInfo const& region)
    -> bool {
  let presents_area = aoc::ranges::accumulate(
      region.counts | stdv::enumerate | stdv::transform([&](auto&& pair) {
        let & [ index, count ] = pair;
        return count * shape_areas[index];
      }),
      usize{});
  return presents_area <= region.width * region.height;
}

fn solve_case1(Input const& input) -> usize {
  let & [ shape_areas, regions ] = input;
  return static_cast<usize>(
      stdr::count_if(regions, [&](RegionInfo const& region) {
        return can_fit_into_region(shape_areas, region);
      }));
}

int main() {
  std::println("Part 1");
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(510, solve_case1(input));

  AOC_RETURN_CHECK_RESULT();
}
