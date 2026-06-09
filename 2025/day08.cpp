// https://adventofcode.com/2025/day/8

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <limits>
#include <print>
#include <ranges>
#include <set>

using Boxes = Vec<std::array<i64, 3>>;
constexpr let usize_max = std::numeric_limits<usize>::max();

auto parse(String const& filename) -> Boxes {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return aoc::split_to_array<3, i64>(line, ','); }) |
         aoc::ranges::to<Boxes>();
}

struct DistanceEntry {
  u64 distance;
  usize from;
  usize to;
};

template <usize NUM_CONNECTIONS>
fn solve_case(Boxes const& boxes) -> usize {
  auto distances =
      stdv::cartesian_product(boxes | stdv::enumerate,
                              boxes | stdv::enumerate) |
      stdv::filter([](auto&& ab) {
        let & [ a, b ] = ab;
        return std::get<0>(a) < std::get<0>(b);
      }) |
      stdv::transform([](auto&& ab) {
        let & [ a, b ] = ab;
        let & [ index_p, p ] = a;
        let & [ index_q, q ] = b;
        return DistanceEntry{
            .distance = (static_cast<u64>(aoc::pown(p[0] - q[0], 2)) +
                         static_cast<u64>(aoc::pown(p[1] - q[1], 2)) +
                         static_cast<u64>(aoc::pown(p[2] - q[2], 2))),
            .from = static_cast<usize>(index_p),
            .to = static_cast<usize>(index_q),
        };
      }) |
      aoc::collect_vec<DistanceEntry>();
  stdr::sort(distances, {}, &DistanceEntry::distance);
  // In the beginning each box is its own circuit
  auto circuit_map = Range{0uz, boxes.size()} | aoc::collect_vec<usize>();
  for (let& de :
       distances | stdv::take(std::min(NUM_CONNECTIONS, distances.size()))) {
    let from = circuit_map[de.from];
    let to = circuit_map[de.to];
    if (from == to) {
      // Already in same circuit, do nothing
      continue;
    }
    // Join circuits by renaming old circuit names to new ones
    let old_name = std::max(from, to);
    let new_name = std::min(from, to);
    auto new_count = 0;
    for (auto& current : circuit_map) {
      if (current == old_name) {
        current = new_name;
      }
      if constexpr (NUM_CONNECTIONS == usize_max) {
        new_count += static_cast<usize>(current == new_name);
      }
    }
    if constexpr (NUM_CONNECTIONS == usize_max) {
      if (new_count == boxes.size()) {
        return static_cast<usize>(boxes[de.from][0] * boxes[de.to][0]);
      }
    }
  }
  AOC_ASSERT(NUM_CONNECTIONS < usize_max,
             "This code should be unreachable in part 2");
  // Sum up circuit sizes
  auto circuit_sizes = Vec(boxes.size(), 0);
  for (let circuit : circuit_map) {
    circuit_sizes[circuit] += 1;
  }
  stdr::sort(circuit_sizes, stdr::greater{});
  return circuit_sizes[0] * circuit_sizes[1] * circuit_sizes[2];
}

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT(40, solve_case<10>(example));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT(102816, solve_case<1000>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(25272, solve_case<usize_max>(example));
  AOC_EXPECT_RESULT(100011612, solve_case<usize_max>(input));

  AOC_RETURN_CHECK_RESULT();
}
