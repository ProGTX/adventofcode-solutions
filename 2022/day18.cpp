// https://adventofcode.com/2022/day/18

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>

using Droplet = aoc::nd_point_type<i32, 3>;
using Input = aoc::flat_set<Droplet>;

fn parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return aoc::split<Droplet>(line, ','); }) |
         aoc::ranges::to<Input>();
}

// Coordinates run 0..21 in the input. The exterior fill reaches one cell
// beyond that, and the surface count then probes one cell beyond *that*,
// so the widest span is -2..23 and a +2 offset keeps it in [0, DIM).
// The set is built once and then probed six times per cell,
// which is exactly what a bitmap is for.
constexpr let DIM = 26uz;
using DropletSet = aoc::bitmap_set<usize, DIM * DIM * DIM>;

fn droplet_index(Droplet const& droplet) -> usize {
  let x = static_cast<usize>(droplet[0] + 2);
  let y = static_cast<usize>(droplet[1] + 2);
  let z = static_cast<usize>(droplet[2] + 2);
  AOC_ASSERT((x < DIM) && (y < DIM) && (z < DIM), "Coordinate out of range");
  return (x * DIM + y) * DIM + z;
}

fn to_droplet_set(Input const& droplets) -> DropletSet {
  auto bits = DropletSet{};
  for (let& droplet : droplets) {
    bits.insert(droplet_index(droplet));
  }
  return bits;
}

constexpr let deltas = std::array{
    Droplet{0, 0, 1},  Droplet{0, 1, 0},  Droplet{1, 0, 0},
    Droplet{0, 0, -1}, Droplet{0, -1, 0}, Droplet{-1, 0, 0},
};

fn solve_case1(Input const& droplets) -> u32 {
  let droplet_bits = to_droplet_set(droplets);
  auto area = u32{};
  // For each droplet cube, add 1 for each of its sides
  // if there isn't an existing droplet in that space
  for (let& droplet : droplets) {
    for (let& delta : deltas) {
      area += static_cast<u32>(
          !droplet_bits.contains(droplet_index(droplet + delta)));
    }
  }
  return area;
}

fn solve_case2(Input const& droplets) -> u32 {
  let droplet_bits = to_droplet_set(droplets);
  // Find bounding box extended by 1 in each direction,
  // so the fill can flow around the entire exterior of the droplet cluster
  auto min_pt = *droplets.begin();
  auto max_pt = *droplets.begin();
  for (let& d : droplets) {
    for (int i = 0; i < 3; ++i) {
      min_pt[i] = std::min(min_pt[i], d[i]);
      max_pt[i] = std::max(max_pt[i], d[i]);
    }
  }
  min_pt += Droplet{-1, -1, -1};
  max_pt += Droplet{1, 1, 1};

  let in_bounds = [&](Droplet const& pos) {
    for (int i = 0; i < 3; ++i) {
      if ((pos[i] < min_pt[i]) || (pos[i] > max_pt[i])) {
        return false;
      }
    }
    return true;
  };

  // BFS from the corner of the bounding box outward
  let exterior = aoc::flood_fill<Input>(min_pt, [&](Droplet const& current) {
    return deltas |
           aoc::views::transform_filter([&](Droplet delta) -> Option<Droplet> {
             let neighbor = current + delta;
             if (!in_bounds(neighbor) ||
                 droplet_bits.contains(droplet_index(neighbor))) {
               return None;
             }
             return neighbor;
           });
  });

  // Each exterior air cell's droplet-neighbors are exterior surface faces
  return aoc::ranges::accumulate(
      exterior | stdv::transform([&](Droplet const& current) {
        return static_cast<u32>(
            stdr::count_if(deltas, [&](Droplet const& delta) {
              return droplet_bits.contains(droplet_index(current + delta));
            }));
      }),
      u32{});
}

int main() {
  std::println("Part 1");
  let example = parse("day18.example");
  AOC_EXPECT_RESULT(10, solve_case1(example));
  let example2 = parse("day18.example2");
  AOC_EXPECT_RESULT(64, solve_case1(example2));
  let input = parse("day18.input");
  AOC_EXPECT_RESULT(4310, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(58, solve_case2(example2));
  AOC_EXPECT_RESULT(2466, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
