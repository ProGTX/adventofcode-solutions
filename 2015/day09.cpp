// https://adventofcode.com/2015/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <print>
#include <ranges>

struct link_t {
  int to_id;
  int distance;
};

using connections_t = std::vector<std::vector<link_t>>;

connections_t parse(const std::string& filename) {
  int current_id = 0;
  std::map<std::string, int> place_ids;
  let get_id = [&](str place) {
    let[it, inserted] = place_ids.try_emplace(std::string{place}, current_id);
    if (inserted) {
      ++current_id;
    }
    return it->second;
  };
  connections_t connections;
  for (str line : aoc::views::read_lines(filename)) {
    let[from, to_str, to, eq_str, value] = aoc::split_to_array<5>(line, ' ');
    let from_id = get_id(from);
    let to_id = get_id(to);
    connections.resize(
        std::ranges::max({connections.size(), static_cast<size_t>(from_id + 1),
                          static_cast<size_t>(to_id + 1)}));
    let distance = aoc::to_number<int>(value);
    connections[from_id].emplace_back(to_id, distance);
    connections[to_id].emplace_back(from_id, distance);
  }
  return connections;
}

// This function assumes the graph is fully connected
template <auto compare>
constexpr int best_distance(const connections_t& connections) {
  auto best = []() {
    if constexpr (compare(0, 1)) { // less
      return std::numeric_limits<int>::max();
    } else { // greater
      return 0;
    }
  }();
  auto current_path = aoc::flat_set<int>{};
  let on_path = [&](const int id) { return current_path.contains(id); };
  for (let& [ from_id, place ] : connections | std::views::enumerate) {
    AOC_ASSERT(std::ranges::is_sorted(place, compare, &link_t::distance),
               "Destinations must be sorted by distance");
    auto current_link = place;
    current_path.clear();
    current_path.insert(from_id);
    auto current_distance = 0;
    while (current_path.size() < connections.size()) {
      let it = std::ranges::find_if_not(current_link, on_path, &link_t::to_id);
      AOC_ASSERT(it != std::end(current_link), "Graph must be fully connected");
      current_path.insert(it->to_id);
      current_distance += it->distance;
      current_link = connections[it->to_id];
    }
    if (compare(current_distance, best)) {
      best = current_distance;
    }
  }
  return best;
}

template <auto compare>
int solve_case(const std::string& filename) {
  auto connections = parse(filename);
  for (auto& place : connections) {
    std::ranges::sort(place, compare, &link_t::distance);
  }
  return best_distance<compare>(connections);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(605, solve_case<std::ranges::less{}>("day09.example"));
  AOC_EXPECT_RESULT(141, solve_case<std::ranges::less{}>("day09.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(982, solve_case<std::ranges::greater{}>("day09.example"));
  AOC_EXPECT_RESULT(736, solve_case<std::ranges::greater{}>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
