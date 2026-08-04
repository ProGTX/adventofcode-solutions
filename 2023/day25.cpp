// https://adventofcode.com/2023/day/25

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

using Graph = Vec<Vec<usize>>;

// A directed edge, since flow always runs one way.
// An undirected connection is two arcs, one per direction.
struct GraphArc {
  usize from;
  usize to;

  constexpr bool operator==(GraphArc const&) const = default;

  fn reversed() const -> GraphArc { return {to, from}; }
};
template <>
struct std::hash<GraphArc> : aoc::packed_hash {};
static_assert(aoc::hashable<GraphArc>);

// How many remaining units of flow each arc can still carry
using Residual = aoc::hash_map<GraphArc, i32>;

// The puzzle states the two groups are held together by exactly three wires
constexpr usize CUT_SIZE = 3;

fn parse(String const& filename) -> Graph {
  auto name_to_id = aoc::name_to_id{};
  auto graph = Graph{};
  for (str line : aoc::views::read_lines(filename)) {
    let[name, connections] = aoc::split_once(line, ':');
    let id = name_to_id.intern(aoc::trim(name));
    for (str connection : aoc::split_sstream(connections)) {
      let connection_id = name_to_id.intern(connection);
      let new_size = name_to_id.new_size(graph.size());
      graph.resize(new_size);
      // A connection is listed on one side only, but goes both ways
      graph[id].push_back(connection_id);
      graph[connection_id].push_back(id);
    }
  }
  return graph;
}

// The neighbors still reachable from `node` without a saturated arc
fn open_neighbors(Graph const& graph, Residual const& residual, usize node)
    -> Vec<usize> {
  return graph[node] |
         stdv::filter([&](usize next) {
           return residual.at(GraphArc{node, next}) > 0;
         }) |
         aoc::collect_vec<usize>();
}

// Edmonds-Karp algorithm.
// The largest number of edge-disjoint paths between `source` and `sink`,
// which by Menger's theorem is the smallest number of wires separating them.
// Gives up as soon as the count exceeds `CUT_SIZE`,
// since then the two are on the same side and the exact number is of no use.
// The residual is left as the search found it,
// so a returned `CUT_SIZE` leaves exactly the arcs of the cut saturated.
fn max_flow(Graph const& graph, Residual& residual, usize source, usize sink)
    -> usize {
  auto flow = usize{};
  while (flow <= CUT_SIZE) {
    auto predecessors = aoc::predecessor_map<usize>{};
    // Every arc counts as one step,
    // so this is a BFS for the shortest augmenting path,
    // as Edmonds-Karp prescribes
    let distances = aoc::shortest_distances_dijkstra(
        source, [&](usize node) { return node == sink; },
        [&](usize node) {
          return aoc::dijkstra_uniform_neighbors(
              open_neighbors(graph, residual, node));
        },
        &predecessors);
    if (!distances.contains(sink)) {
      break;
    }
    // `get_path` is in end-to-start order, and leaves out the start node
    auto path = aoc::get_path(predecessors, sink);
    path.push_back(source);
    stdr::reverse(path);
    for (let[from, to] : path | stdv::adjacent<2>) {
      let arc = GraphArc{from, to};
      // Freeing up the arc in the opposite direction
      // is what lets a later path undo this one's choice
      --residual.at(arc);
      ++residual.at(arc.reversed());
    }
    ++flow;
  }
  return flow;
}

fn solve_case(Graph const& graph) -> usize {
  // `parse` put every connection into both nodes' neighbor lists,
  // so one arc per neighbor entry covers both directions of every connection,
  // each starting out unused
  auto full_residual = Residual{};
  for (let node : aoc::views::indices_of(graph)) {
    for (let next : graph[node]) {
      full_residual.emplace(GraphArc{node, next}, 1);
    }
  }

  // Any node will do as the source,
  // but the sink has to end up on the other side of the cut,
  // which only shows in its flow
  let source = usize{0};
  for (let sink : aoc::views::indices_of(graph) |
                      stdv::filter([&](usize s) { return s != source; })) {
    auto residual = full_residual;
    if (max_flow(graph, residual, source, sink) != CUT_SIZE) {
      continue;
    }
    // With the cut saturated, the source can no longer reach past it
    let group = aoc::flood_fill(source, [&](usize node) {
      return open_neighbors(graph, residual, node);
    });
    return group.size() * (graph.size() - group.size());
  }
  AOC_UNREACHABLE("No cut of 3 wires found");
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day25.example");
  AOC_EXPECT_RESULT(54, solve_case(example));
  let input = parse("day25.input");
  AOC_EXPECT_RESULT(601344, solve_case(input));

  std::println("Part 2");
  aoc::return_incomplete();
}
