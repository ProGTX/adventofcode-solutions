// https://adventofcode.com/2023/day/23

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

using island_t = aoc::char_grid<>;

// Contracted island: only junctions (plus the start and the end) are nodes,
// the corridors between them become edges weighted by their length.
using graph_t = Vec<Vec<aoc::dijkstra_neighbor_t<usize>>>;

constexpr usize no_node = std::numeric_limits<usize>::max();

fn parse(const String& filename) -> island_t {
  return aoc::read_char_grid(filename);
}

fn start_pos() -> point { return point{1, 0}; }

fn end_pos(island_t const& island_map) -> point {
  return point{static_cast<i32>(island_map.num_columns()) - 2,
               static_cast<i32>(island_map.num_rows()) - 1};
}

template <bool allow_slope>
fn walkable_neighbors(island_t const& island_map, const point pos)
    -> aoc::static_vector<point, 4> {
  return aoc::basic_neighbor_diffs |
         aoc::views::transform_filter([&](const point diff) -> Option<point> {
           let neighbor = pos + diff;
           if (!island_map.in_bounds(neighbor.y, neighbor.x)) {
             return None;
           }
           let slope = [&](const point direction) -> Option<point> {
             if constexpr (allow_slope) {
               // Slopes are walkable in any direction in part 2
               return neighbor;
             } else {
               // Slopes may only be entered in their direction in part 1,
               // which prevents cycles and keeps the graph a DAG.
               return (diff == direction) ? Option<point>{neighbor} : None;
             }
           };
           switch (island_map.at(neighbor.y, neighbor.x)) {
             case '#':
               return None;
             case '>':
               return slope({1, 0});
             case '<':
               return slope({-1, 0});
             case '^':
               return slope({0, -1});
             case 'v':
               return slope({0, 1});
             default:
               return neighbor;
           }
         }) |
         aoc::collect_static_vec<point, 4>();
}

fn solve_case1(island_t const& island_map) -> u32 {
  let end = end_pos(island_map);

  let distances = aoc::critical_distances(
      start_pos(), [&](const point& pos) { return pos == end; },
      [&](const point& pos) {
        return aoc::dijkstra_uniform_neighbors(
            walkable_neighbors<false>(island_map, pos));
      });

  return distances.at(end);
}

// Follows the corridor leaving a node through `first_step`
// until another node is reached,
// returning that node and the number of steps taken to get there.
// Returns None for a dead end corridor, which carries no useful edge.
fn follow_corridor(island_t const& island_map, const Vec<usize>& node_ids,
                   const point node, const point first_step)
    -> Option<aoc::dijkstra_neighbor_t<usize>> {
  auto previous = node;
  auto current = first_step;
  auto distance = 1;
  // Corridor tiles have exactly two neighbors,
  // so there is never a choice to make:
  // just keep walking away from where we came from.
  while (node_ids[island_map.linear_index(current.y, current.x)] == no_node) {
    let neighbors = walkable_neighbors<true>(island_map, current);
    let it = stdr::find_if(
        neighbors, [&](const point neighbor) { return neighbor != previous; });
    if (it == stdr::end(neighbors)) {
      return None;
    }
    previous = current;
    current = *it;
    ++distance;
  }
  return aoc::dijkstra_neighbor_t<usize>{
      node_ids[island_map.linear_index(current.y, current.x)], distance};
}

// Returns the contracted graph plus the ids of the start and the end node.
fn contract(island_t const& island_map) -> std::tuple<graph_t, usize, usize> {
  let start = start_pos();
  let end = end_pos(island_map);

  auto node_ids =
      Vec<usize>(island_map.num_rows() * island_map.num_columns(), no_node);
  auto nodes = Vec<point>{};
  for (let row : Range{0uz, island_map.num_rows()}) {
    for (let column : Range{0uz, island_map.num_columns()}) {
      if (island_map.at(row, column) == '#') {
        continue;
      }
      let pos = point{static_cast<i32>(column), static_cast<i32>(row)};
      let is_junction = walkable_neighbors<true>(island_map, pos).size() > 2;
      if (is_junction || (pos == start) || (pos == end)) {
        node_ids[island_map.linear_index(row, column)] = nodes.size();
        nodes.push_back(pos);
      }
    }
  }

  auto graph = graph_t(nodes.size());
  for (let& [ id, node ] : nodes | stdv::enumerate) {
    for (let first_step : walkable_neighbors<true>(island_map, node)) {
      if (let edge = follow_corridor(island_map, node_ids, node, first_step)) {
        graph[id].push_back(*edge);
      }
    }
  }

  return {std::move(graph), node_ids[island_map.linear_index(start.y, start.x)],
          node_ids[island_map.linear_index(end.y, end.x)]};
}

fn solve_case2(island_t const& island_map) -> u32 {
  let[graph, start, end] = contract(island_map);

  let longest = aoc::longest_simple_path(
      graph.size(), start,
      [end = end](const usize node, const usize)
          AOC_FORCE_INLINE { return node == end; },
      [&graph](const usize node)
          AOC_FORCE_INLINE -> const auto& { return graph[node]; });

  return static_cast<u32>(longest.value());
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(94, solve_case1(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(2326, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(154, solve_case2(example));
  AOC_EXPECT_RESULT(6574, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
