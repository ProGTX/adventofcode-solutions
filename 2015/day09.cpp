// https://adventofcode.com/2015/day/9

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

// Carcinization
#define let const auto
using str = std::string_view;

struct link_t {
  int to_id;
  int distance;

  friend std::ostream& operator<<(std::ostream& out, const link_t& link) {
    out << std::format("({}, {})", link.to_id, link.distance);
    return out;
  }
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
    let[from, to_str, to, eq_str, value] =
        aoc::split<std::array<str, 5>>(line, ' ');
    let from_id = get_id(from);
    let to_id = get_id(to);
    connections.resize(
        std::ranges::max({connections.size(), static_cast<size_t>(from_id + 1),
                          static_cast<size_t>(to_id + 1)}));
    let distance = aoc::to_number<int>(value);
    connections[from_id].emplace_back(to_id, distance);
    connections[to_id].emplace_back(from_id, distance);
  }
  for (auto& con : connections) {
    std::ranges::sort(con, std::ranges::less{}, &link_t::distance);
  }
  return connections;
}

struct node_t {
  int from_id;
  int link_index;
  link_t link;
};

constexpr int shortest_distance(const connections_t& connections) {
  auto shortest = std::numeric_limits<int>::max();
  auto current_distance = 0;
  auto current_path = std::vector<node_t>{};
  let on_path = [&](const int id) {
    return std::ranges::contains(current_path, id, &node_t::from_id);
  };
  let backtrack_path = [&]() {
    while (!current_path.empty()) {
      auto& current_place = current_path.back();
      // Backtrack distance we traveled to here
      current_distance -= current_place.link.distance;
      // Find a new destination that we haven't visited yet
      let& possible_destinations = connections[current_place.from_id];
      let it = std::ranges::find_if_not(
          possible_destinations |
              std::views::take(current_place.link_index + 1),
          on_path, &link_t::to_id);
      if (it == std::end(possible_destinations)) {
        // No destination was OK, try going back one step
        current_path.pop_back();
      } else {
        // Found a new path base
        current_place.link = *it;
        break;
      }
    }
  };
  for (let[from_id, place] : connections | std::views::enumerate) {
    current_path.clear();
    current_path.emplace_back(from_id, 0, place[0]);
    current_distance = 0;
    while (!current_path.empty()) {
      let& current_place = current_path.back();
      current_distance += current_place.link.distance;
      if (current_distance >= shortest) {
        backtrack_path();
        continue;
      }
      let full_path = current_path.size() == (connections.size() - 1);
      if (full_path) {
        if (current_distance < shortest) {
          shortest = current_distance;
        }
        backtrack_path();
      } else {
        let& possible_destinations = connections[current_place.link.to_id];
        let it = std::ranges::find_if_not(possible_destinations, on_path,
                                          &link_t::to_id);
        if (it == std::end(possible_destinations)) {
          // Cannot travel any more
          break;
        }
        let index = std::distance(std::begin(possible_destinations), it);
        current_path.emplace_back(current_place.link.to_id, index,
                                  possible_destinations[index]);
      }
    }
  }
  return shortest;
}

int solve_case1(const std::string& filename) {
  let connections = parse(filename);
  for (let& [ index, connection ] : connections | std::views::enumerate) {
    std::cout << index << ": " << aoc::print_range(connection) << std::endl;
  }
  return 0;
  // return shortest_distance(connections);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(605, solve_case1("day09.example"));
  AOC_EXPECT_RESULT(1333, solve_case1("day09.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(19, solve_case2("day09.example"));
  // AOC_EXPECT_RESULT(2046, solve_case2("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
