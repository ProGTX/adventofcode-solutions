// https://adventofcode.com/2023/day/17

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <compare>
#include <iostream>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using city_block_t = aoc::grid<int>;

struct node_t {
  int heat_loss;
  point pos;
  point direction;
  int consecutive;

  friend std::ostream& operator<<(std::ostream& out, const node_t& node) {
    out << "node_t{" << node.pos << "," << node.direction << ","
        << node.consecutive << "," << node.heat_loss << "}";
    return out;
  }

  constexpr std::weak_ordering operator<=>(const node_t& other) const = default;
};

struct visited_node_comparator {
  constexpr bool operator()(const node_t& lhs, const node_t& rhs) const {
    return std::tie(lhs.pos, lhs.direction, lhs.consecutive) <
           std::tie(rhs.pos, rhs.direction, rhs.consecutive);
  }
};

template <int min, int max>
int least_heat_loss(const city_block_t& city_block, const node_t start) {
  std::set<node_t, visited_node_comparator> visited;
  std::set<node_t> unvisited;
  {
    // Push two options as the starting nodes
    node_t node = start;
    node.direction = aoc::get_diff(aoc::east);
    unvisited.insert(node);
    node.direction = aoc::get_diff(aoc::south);
    unvisited.insert(node);
  }

  const auto add_neighbor = [&](node_t node, point direction, int consecutive) {
    node.pos += direction;
    if (!city_block.in_bounds(node.pos.y, node.pos.x)) {
      return;
    }
    node.direction = direction;
    node.consecutive = consecutive;
    node.heat_loss += city_block.at(node.pos.y, node.pos.x);
    if (visited.contains(node) || unvisited.contains(node)) {
      return;
    }
    unvisited.insert(node);
  };

  const auto end_pos =
      point(city_block.num_columns() - 1, city_block.num_rows() - 1);

  city_block_t visited_block{city_block};

  while (!unvisited.empty()) {
    auto current_it = unvisited.begin();
    const auto current = *current_it;
    unvisited.erase(current_it);
    AOC_ASSERT(current.consecutive >= 1, "Invalid consecutive number");
    // aoc::println("current", current);
    // aoc::println("  visited, unvisited", visited.size(), unvisited.size());
    if ((current.consecutive >= min) && (current.pos == end_pos)) {
      return current.heat_loss;
    }
    visited.insert(current);
    // visited_block.modify(0, current.pos.y, current.pos.x);

    auto direction = current.direction;
    if (current.consecutive < max) {
      // Straight line
      add_neighbor(current, direction, current.consecutive + 1);
    }
    if (current.consecutive >= min) {
      // mirror_left
      std::swap(direction.x, direction.y);
      add_neighbor(current, direction, 1);
      // mirror_right
      direction = -direction;
      add_neighbor(current, direction, 1);
    }
  }
  return 0;
}

template <int min, int max>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  city_block_t city_block;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    city_block.add_row(line | aoc::views::to_number<int>());
  }

  int sum = 0;
  sum = least_heat_loss<min, max>(
      city_block,
      node_t{.heat_loss = 0, .pos = {}, .direction = {}, .consecutive = 1});
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(102, (solve_case<1, 3>("day17.example")));
  AOC_EXPECT_RESULT(967, (solve_case<1, 3>("day17.input")));
  // std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(94, (solve_case<4, 10>("day17.example")));
  AOC_EXPECT_RESULT(71, (solve_case<4, 10>("day17.example2")));
  AOC_EXPECT_RESULT(-1105, (solve_case<4, 10>("day17.input")));
  AOC_RETURN_CHECK_RESULT();
}
