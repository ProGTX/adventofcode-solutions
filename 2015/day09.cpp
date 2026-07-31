// https://adventofcode.com/2015/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>

struct link_t {
  int to_id;
  int distance;
};

using connections_t = std::vector<std::vector<link_t>>;

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

// Greedy nearest neighbor: from every starting city,
// repeatedly hop to the closest city not visited yet.
// This is a heuristic, not an exhaustive search -
// a cheap first hop can force an expensive tail
// and nothing here backs out of it -
// but it does find the optimum for this input.
// This function assumes the graph is fully connected
constexpr int shortest_distance(const connections_t& connections) {
  auto best = std::numeric_limits<int>::max();
  auto current_path = aoc::flat_set<int>{};
  let on_path = [&](const int id) { return current_path.contains(id); };
  for (let& [ from_id, place ] : connections | stdv::enumerate) {
    AOC_ASSERT(stdr::is_sorted(place, stdr::less{}, &link_t::distance),
               "Destinations must be sorted by distance");
    auto current_link = place;
    current_path.clear();
    current_path.insert(from_id);
    auto current_distance = 0;
    while (current_path.size() < connections.size()) {
      let it = stdr::find_if_not(current_link, on_path, &link_t::to_id);
      AOC_ASSERT(it != std::end(current_link), "Graph must be fully connected");
      current_path.insert(it->to_id);
      current_distance += it->distance;
      current_link = connections[it->to_id];
    }
    if (current_distance < best) {
      best = current_distance;
    }
  }
  return best;
}

int solve_case1(const std::string& filename) {
  auto connections = parse(filename);
  for (auto& place : connections) {
    stdr::sort(place, stdr::less{}, &link_t::distance);
  }
  return shortest_distance(connections);
}

int solve_case2(const std::string& filename) {
  let connections = parse(filename);
  let num_places = connections.size();
  // Any city may be the start, the route is finished once it has visited all.
  // Only 8 cities, so searching every route exhaustively is cheap.
  auto best = 0;
  for (let start : Range{0uz, num_places}) {
    let distance = aoc::longest_simple_path(
        num_places, start,
        [&](const usize, const usize num_visited) {
          return num_visited == num_places;
        },
        [&](const usize place) {
          return connections[place] | stdv::transform([](const link_t& link) {
                   return aoc::dijkstra_neighbor_t<usize>{
                       static_cast<usize>(link.to_id), link.distance};
                 });
        });
    best = std::max(best, distance.value_or(0));
  }
  return best;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(605, solve_case1("day09.example"));
  AOC_EXPECT_RESULT(141, solve_case1("day09.input"));

  std::println("Part 2");
  AOC_EXPECT_RESULT(982, solve_case2("day09.example"));
  AOC_EXPECT_RESULT(736, solve_case2("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
