#ifndef AOC_ALGORITHM_H
#define AOC_ALGORITHM_H

#include "flat.h"
#include "point.h"
#include "ranges.h"

#include <array>
#include <compare>
#include <concepts>
#include <iostream>
#include <span>
#include <utility>

namespace aoc {

// Specifies a neighbor of the current node
// along with the cost (distance) of reaching it from the current node
template <class Node>
struct dijkstra_neighbor_t {
  Node node;
  int distance;

  constexpr bool operator==(const dijkstra_neighbor_t&) const = default;

  constexpr std::weak_ordering operator<=>(
      const dijkstra_neighbor_t& other) const {
    if (distance == other.distance) {
      return node <=> other.node;
    } else {
      return distance <=> other.distance;
    }
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const dijkstra_neighbor_t& neighbor) {
    out << neighbor.node << " (" << neighbor.distance << ")";
    return out;
  }
};

template <class Node>
using predecessor_map = flat_map<Node, Node>;

// https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#Algorithm
// NOTE: We basically use a priority queue, but use the flat_set for that,
//       because std::priority_queue is still not constexpr in C++23.
//       That's also why we sort by std::greater instead of std::less.
template <class Node, class NeighborsFn>
  requires std::totally_ordered<Node> && requires(Node node) {
    {
      *std::begin(std::declval<NeighborsFn>()(node))
    } -> std::convertible_to<dijkstra_neighbor_t<Node>>;
  }
constexpr flat_map<Node, int> shortest_distances_dijkstra(
    std::span<const Node> start_nodes, std::span<const Node> end_nodes,
    NeighborsFn&& get_reachable_neighbors,
    predecessor_map<Node>* predecessors_out = nullptr) {
  const bool use_predecessors = (predecessors_out != nullptr);

  // 2. Assign to every node a tentative distance value:
  // set it to zero for our initial node and to infinity for all other nodes.
  // During the run of the algorithm,
  // the tentative distance of a node v is the length of the shortest path
  // discovered so far between the node v and the starting node.
  // NOTE: This also serves as the set of visited nodes.
  //       If it hasn't been visited yet, it has an infinite distance.
  // NOTE: The actual sorting direction doesn't matter here, it's just a map.
  // NOTE: The order of steps is reversed because the set of unvisited
  //       relies on distances.
  flat_map<Node, int> distances;
  for (const auto& node : start_nodes) {
    distances.emplace(node, 0);
  }

  // 1. Create a set of all unvisited nodes
  // NOTE: std::greater because it's a priority queue
  //       and we want to store the smallest value at the end
  // TODO: Find a better data structure for this job
  const auto unvisited_compare = [&](const Node& lhs, const Node& rhs) {
    return distances[lhs] > distances[rhs];
  };
  auto unvisited =
      flat_set<Node, decltype(unvisited_compare)>{unvisited_compare};
  for (const auto& node : start_nodes) {
    unvisited.emplace(node);
  }

  auto visited_end_nodes = aoc::flat_set<Node>{};

  while (!unvisited.empty()) {
    // 3. From the unvisited set, select the current node to be the one
    // with the smallest (finite) distance
    // NOTE: We use a priority queue, so we take from the end
    auto current_it = std::end(unvisited);
    --current_it;
    Node current = *current_it;
    auto distance = distances[current];

    // 3. If the only concern is the path to a target node,
    // the algorithm terminates once the current node is the target node.
    if (aoc::ranges::contains(end_nodes, current)) {
      visited_end_nodes.insert(current);
      if (visited_end_nodes.size() == end_nodes.size()) {
        break;
      }
    }

    // 5. After considering all of the current node's unvisited neighbors,
    // the current node is removed from the unvisited set
    // NOTE: We want to remove this early because of the priority queue
    unvisited.erase(current_it);

    // 4. For the current node, consider all of its unvisited neighbors
    // and update their distances through the current node
    auto neighbors = get_reachable_neighbors(current);
    for (const auto& neighbor : neighbors) {
      const auto new_neighbor_dist = distance + neighbor.distance;
      auto existing_it = distances.find(neighbor.node);
      if (existing_it != std::end(distances)) {
        // Neighbor already visited, update the distance
        const auto neighbor_dist = existing_it->second;
        if (new_neighbor_dist < neighbor_dist) {
          distances.erase(existing_it);
          distances.try_emplace(neighbor.node, new_neighbor_dist);
          if (use_predecessors) {
            (*predecessors_out)[neighbor.node] = current;
          }
        }
      } else {
        // Neighbor hasn't been visited yet
        distances.try_emplace(neighbor.node, new_neighbor_dist);
        unvisited.emplace(neighbor.node);
        if (use_predecessors) {
          (*predecessors_out)[neighbor.node] = current;
        }
      }
    }
  }

  return distances;
}

template <class Node, class NeighborsFn>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    predecessor_map<std::remove_cvref_t<Node>>* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::span<const node_t>{},
      std::forward<NeighborsFn>(get_reachable_neighbors), predecessors_out);
}
template <class Node, class NeighborsFn>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, Node&& end_node, NeighborsFn&& get_reachable_neighbors,
    predecessor_map<std::remove_cvref_t<Node>>* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::span<const node_t>{std::array{std::forward<Node>(end_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors), predecessors_out);
}
template <class Node, class NeighborsFn>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, std::span<const std::remove_cvref_t<Node>> end_nodes,
    NeighborsFn&& get_reachable_neighbors,
    predecessor_map<std::remove_cvref_t<Node>>* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      end_nodes, std::forward<NeighborsFn>(get_reachable_neighbors),
      predecessors_out);
}
template <class Node, class NeighborsFn>
constexpr auto shortest_distances_dijkstra(
    std::span<const std::remove_cvref_t<Node>> start_nodes, Node&& end_node,
    NeighborsFn&& get_reachable_neighbors,
    predecessor_map<std::remove_cvref_t<Node>>* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      start_nodes,
      std::span<const node_t>{std::array{std::forward<Node>(end_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors), predecessors_out);
}

} // namespace aoc

#endif // AOC_ALGORITHM_H
