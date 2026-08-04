// https://adventofcode.com/2015/day/9

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#endif

struct link_t {
  int to_id;
  int distance;
};

// There are exactly this many connections
// from one city to all other cities in the input data
constexpr usize MAX_LINKS = 7;

using links_t = aoc::static_vector<link_t, MAX_LINKS>;
using connections_t = std::vector<links_t>;

connections_t parse(const std::string& filename) {
  auto connections = connections_t{};
  auto name_to_id = aoc::name_to_id{};
  for (str line : aoc::views::read_lines(filename)) {
    let[from, to_str, to, eq_str, value] = aoc::split_to_array<5>(line, ' ');
    let from_id = name_to_id.intern(from);
    let to_id = name_to_id.intern(to);
    connections.resize(name_to_id.new_size(connections.size()));
    let distance = aoc::to_number<int>(value);
    connections[from_id].emplace_back(to_id, distance);
    connections[to_id].emplace_back(from_id, distance);
  }
  return connections;
}

// Only the longest route is searched for directly.
// Every complete route visits every city,
// so it always has exactly `num_cities - 1` edges -
// which means the shortest route is the longest one
// over the complemented weights (max_distance - distance),
// and its real length is `(num_cities - 1) * max_distance` minus what's found.
template <bool LONGEST>
int solve_case(const connections_t& connections) {
  let num_cities = connections.size();
  let max_distance =
      stdr::max(connections | stdv::join | stdv::transform(&link_t::distance));
  // Any city may be the start, the route is finished once it has visited all.
  // Only 8 cities, so searching every route exhaustively is cheap.
  auto best = 0;
  for (let start : Range{0uz, num_cities}) {
    let distance = aoc::longest_simple_path(
        num_cities, start,
        [&](const usize, const usize num_visited) {
          return num_visited == num_cities;
        },
        [&](const usize city) {
          return connections[city] | stdv::transform([&](const link_t& link) {
                   return aoc::dijkstra_neighbor_t<usize>{
                       .node = static_cast<usize>(link.to_id),
                       .distance = LONGEST ? link.distance
                                           : max_distance - link.distance};
                 });
        });
    best = std::max(best, distance.value_or(0));
  }
  if constexpr (LONGEST) {
    return best;
  } else {
    return static_cast<int>(num_cities - 1) * max_distance - best;
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(605, solve_case<false>(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(141, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(982, solve_case<true>(example));
  AOC_EXPECT_RESULT(736, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
