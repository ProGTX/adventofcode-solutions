// https://adventofcode.com/2023/day/17

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <compare>
#include <print>
#include <span>
#include <unordered_map>

using city_block_t = aoc::grid<int>;

struct node_t {
  point pos;
  point direction;
  int consecutive;

  constexpr bool operator==(node_t const&) const = default;
  constexpr auto operator<=>(node_t const&) const = default;
};

template <>
struct std::hash<node_t> {
  constexpr size_t operator()(node_t const& node) const {
    auto combine = aoc::hash_combine{};
    combine(std::hash<point>{}(node.pos));
    combine(std::hash<point>{}(node.direction));
    combine(std::hash<int>{}(node.consecutive));
    return combine.seed;
  }
};

fn parse(String const& filename) -> city_block_t {
  auto city_block = city_block_t{};
  for (str line : aoc::views::read_lines(filename)) {
    city_block.add_row(line | aoc::views::to_number<int>());
  }
  return city_block;
}

template <int min, int max>
fn least_heat_loss(city_block_t const& city_block) -> int {
  let end_pos = point(city_block.num_columns() - 1, city_block.num_rows() - 1);

  auto start_nodes = std::array{
      node_t{
          .pos = {}, .direction = aoc::get_diff(aoc::east), .consecutive = 0},
      node_t{
          .pos = {}, .direction = aoc::get_diff(aoc::south), .consecutive = 0},
  };

  let get_neighbors = [&](node_t const& current) {
    auto neighbors = aoc::static_vector<aoc::dijkstra_neighbor_t<node_t>, 3>{};
    let add_neighbor = [&](point direction, int consecutive) {
      let pos = current.pos + direction;
      if (!city_block.in_bounds(pos.y, pos.x)) {
        return;
      }
      neighbors.emplace_back(
          node_t{
              .pos = pos, .direction = direction, .consecutive = consecutive},
          city_block.at(pos.y, pos.x));
    };

    if (current.consecutive < max) {
      // Straight line
      add_neighbor(current.direction, current.consecutive + 1);
    }
    if (current.consecutive >= min) {
      auto direction = current.direction;
      // mirror_left
      std::swap(direction.x, direction.y);
      add_neighbor(direction, 1);
      add_neighbor(-direction, 1);
    }
    return neighbors;
  };

  let end_reached = [&](node_t const& node) {
    return (node.pos == end_pos) && (node.consecutive >= min);
  };

  let distances =
      aoc::shortest_distances_dijkstra<std::unordered_map<node_t, int>>(
          std::span<node_t>{start_nodes}, end_reached, get_neighbors);

  return stdr::min(
      distances |
      stdv::filter([&](let& entry) { return end_reached(entry.first); }) |
      stdv::transform([](let& entry) { return entry.second; }));
}

int main() {
  std::println("Part 1");
  let example = parse("day17.example");
  AOC_EXPECT_RESULT(102, (least_heat_loss<1, 3>(example)));
  let input = parse("day17.input");
  AOC_EXPECT_RESULT(967, (least_heat_loss<1, 3>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(94, (least_heat_loss<4, 10>(example)));
  let example2 = parse("day17.example2");
  AOC_EXPECT_RESULT(71, (least_heat_loss<4, 10>(example2)));
  AOC_EXPECT_RESULT(1101, (least_heat_loss<4, 10>(input)));

  AOC_RETURN_CHECK_RESULT();
}
