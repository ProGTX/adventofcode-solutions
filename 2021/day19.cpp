// https://adventofcode.com/2021/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <future>
#include <map>
#include <print>
#include <set>
#include <span>
#include <string>
#include <thread>

static constexpr usize max_scanners = 32;
static constexpr usize max_beacons = 32;

using Point3D = aoc::nd_point_type<i32, 3>;
using Scanner = aoc::static_vector<Point3D, max_beacons>;

fn parse(String const& filename) -> aoc::static_vector<Scanner, max_scanners> {
  auto scanners = aoc::static_vector<Scanner, max_scanners>{};
  auto current = Scanner{};
  for (str line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.starts_with("---")) {
      current.clear();
    } else if (line.empty()) {
      scanners.push_back(std::move(current));
    } else {
      current.push_back(aoc::split<Point3D>(line, ','));
    }
  }
  // last scanner has no trailing blank line
  scanners.push_back(std::move(current));
  return scanners;
}

// Returns all 24 rotation-distinct orientations of a scanner's beacon list.
// 6 axis permutations times 4 sign combos where det(rotation) = +1.
fn rotations(Scanner const& scanner) -> aoc::static_vector<Scanner, 24> {
  using Transform = Point3D (*)(Point3D);
  static constexpr let transforms = std::array<Transform, 24>{{
      // permutation (x,y,z), det=+1 -> signs product must be +1
      [](Point3D p) -> Point3D { return {p.x(), p.y(), p.z()}; },
      [](Point3D p) -> Point3D { return {p.x(), -p.y(), -p.z()}; },
      [](Point3D p) -> Point3D { return {-p.x(), p.y(), -p.z()}; },
      [](Point3D p) -> Point3D { return {-p.x(), -p.y(), p.z()}; },
      // permutation (x,z,y), det=-1 -> signs product must be -1
      [](Point3D p) -> Point3D { return {p.x(), p.z(), -p.y()}; },
      [](Point3D p) -> Point3D { return {p.x(), -p.z(), p.y()}; },
      [](Point3D p) -> Point3D { return {-p.x(), p.z(), p.y()}; },
      [](Point3D p) -> Point3D { return {-p.x(), -p.z(), -p.y()}; },
      // permutation (y,x,z), det=-1 -> signs product must be -1
      [](Point3D p) -> Point3D { return {p.y(), p.x(), -p.z()}; },
      [](Point3D p) -> Point3D { return {p.y(), -p.x(), p.z()}; },
      [](Point3D p) -> Point3D { return {-p.y(), p.x(), p.z()}; },
      [](Point3D p) -> Point3D { return {-p.y(), -p.x(), -p.z()}; },
      // permutation (y,z,x), det=+1 -> signs product must be +1
      [](Point3D p) -> Point3D { return {p.y(), p.z(), p.x()}; },
      [](Point3D p) -> Point3D { return {p.y(), -p.z(), -p.x()}; },
      [](Point3D p) -> Point3D { return {-p.y(), p.z(), -p.x()}; },
      [](Point3D p) -> Point3D { return {-p.y(), -p.z(), p.x()}; },
      // permutation (z,x,y), det=+1 -> signs product must be +1
      [](Point3D p) -> Point3D { return {p.z(), p.x(), p.y()}; },
      [](Point3D p) -> Point3D { return {p.z(), -p.x(), -p.y()}; },
      [](Point3D p) -> Point3D { return {-p.z(), p.x(), -p.y()}; },
      [](Point3D p) -> Point3D { return {-p.z(), -p.x(), p.y()}; },
      // permutation (z,y,x), det=-1 -> signs product must be -1
      [](Point3D p) -> Point3D { return {p.z(), p.y(), -p.x()}; },
      [](Point3D p) -> Point3D { return {p.z(), -p.y(), p.x()}; },
      [](Point3D p) -> Point3D { return {-p.z(), p.y(), p.x()}; },
      [](Point3D p) -> Point3D { return {-p.z(), -p.y(), -p.x()}; },
  }};
  auto result = aoc::static_vector<Scanner, 24>{};
  for (let& transform : transforms) {
    result.push_back(
        scanner | stdv::transform(transform) | aoc::ranges::to<Scanner>());
  }
  return result;
}

// Try to align `scanner` against the already-known beacon cloud.
// For each of the 24 rotations, count how often each candidate translation
// offset (known_beacon - rotated_beacon) appears.
// If any offset is seen >=12 times, at least 12 beacons coincide,
// so the scanners overlap.
// Return the rotated+translated beacons in world coordinates
// and the scanner's world-space position, or None if no match found.
fn try_align(std::set<Point3D> const& all_beacons, Scanner const& scanner)
    -> Option<std::pair<Scanner, Point3D>> {
  for (let& rotated : rotations(scanner)) {
    auto offset_counts = std::map<Point3D, usize>{};
    for (let& a : all_beacons) {
      for (let& b : rotated) {
        offset_counts[a - b]++;
      }
    }
    let it =
        stdr::find_if(offset_counts, [](let& kv) { return kv.second >= 12; });
    if (it != offset_counts.end()) {
      let offset = it->first;
      auto aligned = rotated |
                     stdv::transform([&offset](Point3D p) -> Point3D {
                       return p + offset;
                     }) |
                     aoc::ranges::to<Scanner>();
      return std::pair{aligned, offset};
    }
  }
  return None;
}

// Repeatedly try to align each unaligned scanner
// against the growing beacon cloud.
// Each successful match merges that scanner's beacons into the cloud and
// removes it from the unaligned set.
// Scanners that can't yet match will eventually match
// once an intermediate scanner has been merged.
// Unaligned scanners are tried in parallel, chunked by hardware concurrency.
fn align_all(std::span<Scanner const> scanners)
    -> std::pair<std::set<Point3D>, aoc::static_vector<Point3D, max_scanners>> {
  auto all_beacons = std::set<Point3D>{scanners[0].begin(), scanners[0].end()};
  auto scanner_positions = aoc::static_vector<Point3D, max_scanners>{{0, 0, 0}};
  auto unaligned = aoc::static_vector<u8, max_scanners>{};
  for (usize i = 1; i < scanners.size(); ++i) {
    unaligned.push_back(static_cast<u8>(i));
  }

  constexpr let max_parallelism = 16;
  let parallelism = std::min<usize>(
      max_parallelism, std::max(1u, std::thread::hardware_concurrency()));
  while (!unaligned.empty()) {
    using AlignResult = std::pair<u8, std::pair<Scanner, Point3D>>;
    auto found = Option<AlignResult>{};

    for (usize chunk_start = 0; chunk_start < unaligned.size() && !found;
         chunk_start += parallelism) {
      let chunk_end = std::min(chunk_start + parallelism, unaligned.size());
      using FutureT = std::future<Option<std::pair<Scanner, Point3D>>>;
      auto futures = aoc::static_vector<FutureT, max_parallelism>{};
      for (usize i = chunk_start; i < chunk_end; ++i) {
        let s = unaligned[i];
        futures.push_back(
            std::async(std::launch::async, [&all_beacons, &scanners, s] {
              return try_align(all_beacons, scanners[s]);
            }));
      }
      for (usize fi = 0; fi < futures.size() && !found; ++fi) {
        if (auto r = futures[fi].get()) {
          found = AlignResult{unaligned[chunk_start + fi], std::move(*r)};
        }
      }
    }

    AOC_ASSERT(found.has_value(), "No scanner aligned this round");

    let s = found->first;
    let& aligned = found->second.first;
    let pos = found->second.second;
    all_beacons.insert(aligned.begin(), aligned.end());
    scanner_positions.push_back(pos);
    unaligned.erase(stdr::find(unaligned, s));
  }

  return {std::move(all_beacons), std::move(scanner_positions)};
}

fn solve_case1(std::span<Scanner const> scanners) -> usize {
  return align_all(scanners).first.size();
}

fn solve_case2(std::span<Scanner const> scanners) -> i32 {
  let positions = align_all(scanners).second;
  auto max_dist = i32{0};
  for (usize i = 0; i < positions.size(); ++i) {
    for (usize j = i + 1; j < positions.size(); ++j) {
      max_dist =
          std::max(max_dist, distance_manhattan(positions[i], positions[j]));
    }
  }
  return max_dist;
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(79, solve_case1(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(372, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3621, solve_case2(example));
  AOC_EXPECT_RESULT(12241, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
