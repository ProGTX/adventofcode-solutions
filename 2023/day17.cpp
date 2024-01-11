// https://adventofcode.com/2023/day/17

#include "../common/common.h"

#include <algorithm>
#include <array>
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
  point pos;
  point direction;
  int consecutive;
  int heat_loss;

  constexpr bool equivalent(const node_t& other) const {
    return std::tie(pos, direction, consecutive) ==
           std::tie(other.pos, other.direction, other.consecutive);
  }

  friend std::ostream& operator<<(std::ostream& out, const node_t& node) {
    out << "node_t{" << node.pos << "," << node.direction << ","
        << node.consecutive << "," << node.heat_loss << "}";
    return out;
  }
};

struct visited_node_comparator {
  constexpr bool operator()(const node_t& lhs, const node_t& rhs) const {
    return std::tie(lhs.pos, lhs.direction, lhs.consecutive) <
           std::tie(rhs.pos, rhs.direction, rhs.consecutive);
  }
};

int least_heat_loss(const city_block_t& city_block, const node_t start) {
  std::set<node_t, visited_node_comparator> visited;
  std::vector<node_t> unvisited;
  {
    // Push two options as the starting nodes
    node_t node = start;
    node.direction = aoc::get_diff(aoc::east);
    unvisited.push_back(node);
    node.direction = aoc::get_diff(aoc::south);
    unvisited.push_back(node);
  }

  const auto add_neighbor = [&](node_t node, point direction, int consecutive) {
    node.pos += direction;
    if (!city_block.in_bounds(node.pos.y, node.pos.x)) {
      return;
    }
    node.direction = direction;
    node.consecutive = consecutive;
    if (visited.contains(node)) {
      return;
    }
    if (std::ranges::find_if(unvisited, [&](const node_t& unv) {
          return node.equivalent(unv);
        }) != unvisited.end()) {
      return;
    }
    node.heat_loss += city_block.at(node.pos.y, node.pos.x);
    unvisited.push_back(std::move(node));
  };

  const auto end_pos =
      point(city_block.num_columns() - 1, city_block.num_rows() - 1);

  city_block_t visited_block{city_block};

  while (!unvisited.empty()) {
    const auto current = aoc::pop_stack(unvisited);
    // aoc::println("current", current);
    // aoc::println("  visited, unvisited", visited.size(), unvisited.size());
    if (current.pos == end_pos) {
      return current.heat_loss;
    }
    visited.insert(current);
    // visited_block.modify(0, current.pos.y, current.pos.x);

    auto direction = current.direction;
    if (current.consecutive < 3) {
      // Straight line
      add_neighbor(current, direction, current.consecutive + 1);
    }
    // mirror_left
    std::swap(direction.x, direction.y);
    add_neighbor(current, direction, 1);
    // mirror_right
    direction = -direction;
    add_neighbor(current, direction, 1);

    std::ranges::sort(unvisited, [](const node_t& lhs, const node_t& rhs) {
      // Nodes with the least heat loss are at the top of the stack,
      // which is the end of the vector
      return lhs.heat_loss > rhs.heat_loss;
    });
  }
  return 0;
}

template <bool>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  city_block_t city_block;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    city_block.add_row(line | aoc::views::to_number<int>());
  }

  int sum = 0;
  sum = least_heat_loss(
      city_block,
      node_t{.pos = {}, .direction = {}, .consecutive = 1, .heat_loss = 0});
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(102, (solve_case<false>("day17.example")));
  AOC_EXPECT_RESULT(967, (solve_case<false>("day17.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(51, (solve_case<true>("day17.example")));
  // AOC_EXPECT_RESULT(8026, (solve_case<true>("day17.input")));
  AOC_RETURN_CHECK_RESULT();
}
