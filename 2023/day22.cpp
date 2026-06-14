// https://adventofcode.com/2023/day/22

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>

using coord_t = aoc::nd_point_type<int, 3>;
using brick_t = aoc::closed_range<coord_t>;
using brick_list_t = Vec<brick_t>;

fn parse(const std::string& filename) -> brick_list_t {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let[first, last] = aoc::split_once(line, '~');
           return brick_t{aoc::split<coord_t>(first, ','),
                          aoc::split<coord_t>(last, ',')};
         }) |
         aoc::collect_vec<brick_t>();
}

// Returns whether two bricks overlap when viewed from above.
fn overlaps_xy(const brick_t& a, const brick_t& b) -> bool {
  return (a.begin.x() <= b.end.x()) &&
         (a.end.x() >= b.begin.x()) &&
         (a.begin.y() <= b.end.y()) &&
         (a.end.y() >= b.begin.y());
}

// Drops each brick straight down until it rests on the floor
// or on top of an already-settled brick below it.
// Bricks are processed lowest-first,
// so by the time a brick is settled,
// every brick that could support it is already in its final position.
fn fall_down(brick_list_t& bricks) {
  // Sorts bricks in increasing order by their lowest z component
  stdr::sort(bricks, {}, [](const brick_t& brick) { return brick.begin.z(); });

  for (let i : Range{0uz, bricks.size()}) {
    auto& brick = bricks[i];
    let height = brick.end.z() - brick.begin.z();

    // Find the highest surface among the already-settled bricks
    // below this one that it would land on, or the floor
    auto new_begin_z = 1;
    for (let j : Range{0uz, i}) {
      if (overlaps_xy(brick, bricks[j])) {
        new_begin_z = std::max(new_begin_z, bricks[j].end.z() + 1);
      }
    }

    // Settle the brick directly on top of that surface
    brick.begin.z() = new_begin_z;
    brick.end.z() = new_begin_z + height;
  }
}

// supports[i] lists the bricks resting directly on top of brick i
// supported_by[i] lists the bricks directly below brick i
fn get_supports(const brick_list_t& bricks)
    -> std::pair<Vec<Vec<usize>>, Vec<Vec<usize>>> {
  auto supports = Vec<Vec<usize>>(bricks.size());
  auto supported_by = Vec<Vec<usize>>(bricks.size());

  for (let i : Range{0uz, bricks.size()}) {
    for (let j : Range{0uz, bricks.size()}) {
      if ((i != j) &&
          (bricks[j].begin.z() == bricks[i].end.z() + 1) &&
          overlaps_xy(bricks[i], bricks[j])) {
        supports[i].push_back(j);
        supported_by[j].push_back(i);
      }
    }
  }

  return {std::move(supports), std::move(supported_by)};
}

fn solve_case1(brick_list_t bricks) -> usize {
  fall_down(bricks);
  let[supports, supported_by] = get_supports(bricks);

  return static_cast<usize>(stdr::count_if(supports, [&](let& above) {
    return stdr::all_of(above,
                        [&](usize j) { return supported_by[j].size() > 1; });
  }));
}

fn solve_case2(brick_list_t bricks) -> usize {
  fall_down(bricks);
  let[supports, supported_by] = get_supports(bricks);

  // Simulate disintegrating each brick in turn
  // and count the chain reaction it triggers
  auto total = 0uz;

  for (let i : Range{0uz, bricks.size()}) {
    // Bricks known to have fallen, starting with brick i itself
    auto fallen = Vec<bool>(bricks.size(), false);
    fallen[i] = true;
    // Candidates to check, seeded with everything resting on i
    auto queue = supports[i];

    while (!queue.empty()) {
      let j = queue.back();
      queue.pop_back();

      // Skip bricks already counted, or ones still held up
      // by a support that hasn't fallen (yet).
      // The latter get re-queued and rechecked
      // once that support falls.
      if (fallen[j] ||
          !stdr::all_of(supported_by[j], [&](usize k) { return fallen[k]; })) {
        continue;
      }
      fallen[j] = true;
      ++total;
      // j fell, so bricks resting on it might fall too
      stdr::copy(supports[j], std::back_inserter(queue));
    }
  }

  return total;
}

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(5, solve_case1(example));
  let example2 = parse("day22.example2");
  AOC_EXPECT_RESULT(3, solve_case1(example2));
  let example3 = parse("day22.example3");
  AOC_EXPECT_RESULT(2, solve_case1(example3));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(465, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(7, solve_case2(example));
  AOC_EXPECT_RESULT(1, solve_case2(example2));
  AOC_EXPECT_RESULT(3, solve_case2(example3));
  AOC_EXPECT_RESULT(79042, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
