// https://adventofcode.com/2023/day/24

#include "../common/common.h"
#include "../common/rust.h"

#include <print>

using Vec3 = aoc::nd_point_type<double, 3>;

struct hailstone_t {
  Vec3 position;
  Vec3 velocity;
};

using hailstones_t = Vec<hailstone_t>;

fn parse(const String& filename) -> hailstones_t {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let[pos_str, vel_str] = aoc::split_once(line, '@');
           return hailstone_t{aoc::split<Vec3>(pos_str, ','),
                              aoc::split<Vec3>(vel_str, ',')};
         }) |
         aoc::collect_vec<hailstone_t>();
}

/// Returns the XY intersection point if the paths cross in the future
fn intersect_xy(const hailstone_t& h1, const hailstone_t& h2) -> Option<Vec3> {
  // We want to find time_h1 and time_h2 such that (in XY):
  //   h1.position + time_h1 * h1.velocity = h2.position + time_h2 * h2.velocity
  //
  // Rearranging into a 2x2 linear system Ax = b:
  //   [ h1.vel.x  -h2.vel.x ] [ time_h1 ]   [ h2.pos.x - h1.pos.x ]
  //   [ h1.vel.y  -h2.vel.y ] [ time_h2 ] = [ h2.pos.y - h1.pos.y ]
  //
  // Solved via Cramer's rule
  let determinant =
      h2.velocity.x() * h1.velocity.y() - h1.velocity.x() * h2.velocity.y();
  if (determinant == 0.0) {
    // Parallel paths, no unique intersection
    return std::nullopt;
  }

  let pos_diff = h2.position - h1.position;

  let time_h1 =
      (-pos_diff.x() * h2.velocity.y() + h2.velocity.x() * pos_diff.y()) /
      determinant;
  let time_h2 =
      (h1.velocity.x() * pos_diff.y() - h1.velocity.y() * pos_diff.x()) /
      determinant;

  if (time_h1 < 0.0 || time_h2 < 0.0) {
    // Paths crossed in the past for one or both hailstones
    return std::nullopt;
  }

  // Evaluate h1's position at time_h1 to get the intersection point
  return h1.position + h1.velocity * time_h1;
}

template <u64 MIN, u64 MAX>
fn solve_case1(const hailstones_t& hailstones) -> usize {
  let min = static_cast<double>(MIN);
  let max = static_cast<double>(MAX);
  auto count = 0uz;
  for (let i : Range{0uz, hailstones.size()}) {
    for (let j : Range{i + 1, hailstones.size()}) {
      if (let p = intersect_xy(hailstones[i], hailstones[j])) {
        count += static_cast<usize>((p->x() >= min) &&
                                    (p->x() <= max) &&
                                    (p->y() >= min) &&
                                    (p->y() <= max));
      }
    }
  }
  return count;
}

int main() {
  std::println("Part 1");
  let example = parse("day24.example");
  AOC_EXPECT_RESULT(2, (solve_case1<7, 27>(example)));
  let input = parse("day24.input");
  AOC_EXPECT_RESULT(21785,
                    (solve_case1<200000000000000, 400000000000000>(input)));

  aoc::return_incomplete();
  AOC_RETURN_CHECK_RESULT();
}
