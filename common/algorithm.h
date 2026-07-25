#ifndef AOC_ALGORITHM_H
#define AOC_ALGORITHM_H

#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "string.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <deque>
#include <map>
#include <optional>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

// A map-like container usable for Dijkstra distances:
// keyed on Key, with an int-valued, mutable mapped type.
template <class Container, class Key>
concept distances_map = requires(Container c, const Key& key, int value) {
  typename Container::iterator;
  { c.end() } -> std::same_as<typename Container::iterator>;
  { c.find(key) } -> std::same_as<typename Container::iterator>;
  c.emplace(key, value);
  c.try_emplace(key, value);
  { c.find(key)->second } -> std::convertible_to<int>;
  c.find(key)->second = value;
};

// Specifies a neighbor of the current node
// along with the cost (distance) of reaching it from the current node
template <class Node>
struct dijkstra_neighbor_t {
  Node node;
  int distance;

  constexpr bool operator==(const dijkstra_neighbor_t&) const = default;
  constexpr auto operator<=>(const dijkstra_neighbor_t&) const = default;
};

constexpr auto dijkstra_uniform_neighbors_view() {
  return std::views::transform([](auto&& neighbor) {
    return dijkstra_neighbor_t{std::forward<decltype(neighbor)>(neighbor), 1};
  });
}

template <template <class...> class Primary, class T, class... Args>
constexpr auto dijkstra_uniform_neighbors(Primary<T, Args...>&& neighbors) {
  using Return = reuse_primary_t<Primary, dijkstra_neighbor_t<T>>;
  return neighbors | dijkstra_uniform_neighbors_view() | ranges::to<Return>();
}
template <template <class, auto, class...> class Primary, class T, auto Size,
          class... Args>
constexpr auto dijkstra_uniform_neighbors(
    Primary<T, Size, Args...>&& neighbors) {
  using Return =
      reuse_primary_with_size_t<Primary, dijkstra_neighbor_t<T>, Size>;
  return neighbors | dijkstra_uniform_neighbors_view() | ranges::to<Return>();
}

/// Default associative map for the search algorithms
template <class Key, class T>
using default_map =
    std::conditional_t<hashable<Key>,
                       std::unordered_map<std::remove_cvref_t<Key>, T>,
                       std::map<std::remove_cvref_t<Key>, T>>;

/// Default associative set for the search algorithms
template <class Key>
using default_set =
    std::conditional_t<hashable<Key>,
                       std::unordered_set<std::remove_cvref_t<Key>>,
                       std::set<std::remove_cvref_t<Key>>>;

// Node needs the explicit remove_cvref_t here
// because default_map only strips its key, not its mapped type
// and Node is deduced from a forwarding reference below,
// so it can arrive as an lvalue reference.
template <class Node>
using predecessor_map = default_map<Node, std::remove_cvref_t<Node>>;

// Here we don't need remove_cvref_t, default_set strips its own key
template <class Node>
using predecessor_map_all = default_map<Node, default_set<Node>>;

// https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#Algorithm
// https://en.wikipedia.org/wiki/A*_search_algorithm
// Dijkstra is A* with a zero heuristic (constant_value<int>{}).
// Heuristic must be admissible (never overestimates the actual remaining cost).
template <class ReturnT = void, class Node, class NeighborsFn,
          class HeuristicFn = constant_value<int>,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
  requires std::totally_ordered<Node> &&
           requires(Node node) {
             {
               std::declval<EndReachedFn>()(node)
             } -> std::convertible_to<bool>;
             { std::declval<HeuristicFn>()(node) } -> std::convertible_to<int>;
           } &&
           contains_uncvref<PredecessorMap, predecessor_map<Node>,
                            predecessor_map_all<Node>> &&
           (std::is_void_v<ReturnT> || distances_map<ReturnT, Node>)
constexpr auto shortest_distances_dijkstra(
    std::span<const Node> start_nodes, EndReachedFn&& end_reached,
    NeighborsFn&& get_reachable_neighbors, HeuristicFn&& heuristic = {},
    PredecessorMap* predecessors_out = nullptr) {
  using distances_t = std::conditional_t<std::is_void_v<ReturnT>,
                                         default_map<Node, int>, ReturnT>;

  const bool use_predecessors = (predecessors_out != nullptr);
  constexpr const bool all_predecessors =
      requires(PredecessorMap preds, Node node) { preds[node].emplace(node); };

  // 2. Assign to every node a tentative distance value:
  // set it to zero for our initial node and to infinity for all other nodes.
  // During the run of the algorithm,
  // the tentative distance of a node v is the length of the shortest path
  // discovered so far between the node v and the starting node.
  // NOTE: This also serves as the set of visited nodes.
  //       If it hasn't been visited yet, it has an infinite distance.
  // Distances represent the g values
  auto distances = distances_t{};
  for (const auto& node : start_nodes) {
    distances.emplace(node, 0);
  }

  // 1. Create a set of all unvisited nodes,
  //    more specifically a min-heap of {distance, node} pairs
  // Open set ordered by f = g + h; store g alongside to detect stale entries.
  // With h = 0 this is standard Dijkstra.
  using entry_t = std::tuple<int, int, Node>;
  auto unvisited = priority_queue<entry_t, std::greater<entry_t>>{};
  for (const auto& node : start_nodes) {
    unvisited.emplace(static_cast<int>(heuristic(node)), 0, node);
  }

  while (!unvisited.empty()) {
    // 3. From the unvisited set, select the current node to be the one
    // with the smallest (finite) distance
    auto [f, g_enqueued, current] = unvisited.top();
    unvisited.pop();

    // Skip stale entries — a better g was found after this was enqueued
    if (distances.find(current)->second < g_enqueued) {
      continue;
    }

    // 3. If the only concern is the path to a target node,
    // the algorithm terminates once the current node is the target node.
    if (end_reached(current)) {
      break;
    }

    // 4. For the current node, consider all of its unvisited neighbors
    // and update their distances through the current node
    for (const auto& neighbor : get_reachable_neighbors(current)) {
      const int tentative_g = g_enqueued + neighbor.distance;
      // The insert path below looks the neighbor up twice,
      // which a single try_emplace would fold into one lookup.
      // That was measured across every caller and came out a wash:
      // the second lookup only happens the first time a node is seen,
      // and try_emplace costs more than find when revisiting a known one,
      // which is the common case here.
      auto existing_it = distances.find(neighbor.node);
      if (existing_it != std::end(distances)) {
        // Neighbor already visited, update the distance
        const int neighbor_g = existing_it->second;
        if (tentative_g < neighbor_g) {
          existing_it->second = tentative_g;
          const int f_new =
              tentative_g + static_cast<int>(heuristic(neighbor.node));
          unvisited.emplace(f_new, tentative_g, neighbor.node);
          if constexpr (!all_predecessors) {
            if (use_predecessors) {
              (*predecessors_out)[neighbor.node] = current;
            }
          }
        }
        if constexpr (all_predecessors) {
          if (use_predecessors && (tentative_g <= neighbor_g)) {
            (*predecessors_out)[neighbor.node].emplace(current);
          }
        }
      } else {
        distances.try_emplace(neighbor.node, tentative_g);
        const int f_new =
            tentative_g + static_cast<int>(heuristic(neighbor.node));
        unvisited.emplace(f_new, tentative_g, neighbor.node);
        if (use_predecessors) {
          if constexpr (all_predecessors) {
            (*predecessors_out)[neighbor.node].emplace(current);
          } else {
            (*predecessors_out)[neighbor.node] = current;
          }
        }
      }
    }
  }

  return distances;
}

/// Bidirectional A* for a single start/end pair.
///
/// get_reachable_neighbors walks forward edges,
/// and get_backward_neighbors walks reverse edges.
/// Both neighbor ranges use dijkstra_neighbor_t<Node> costs.
/// Heuristics must be admissible in their respective directions.
template <class Node, class ForwardNeighborsFn, class BackwardNeighborsFn,
          class ForwardHeuristicFn = constant_value<int>,
          class BackwardHeuristicFn = constant_value<int>>
  requires std::totally_ordered<Node> && requires(Node node) {
    { std::declval<ForwardHeuristicFn>()(node) } -> std::convertible_to<int>;
    { std::declval<BackwardHeuristicFn>()(node) } -> std::convertible_to<int>;
  }
constexpr std::optional<int> shortest_distance_bidirectional_astar(
    Node start_node, Node end_node, ForwardNeighborsFn&& get_forward_neighbors,
    BackwardNeighborsFn&& get_backward_neighbors,
    ForwardHeuristicFn&& forward_heuristic = {},
    BackwardHeuristicFn&& backward_heuristic = {}) {
  if (start_node == end_node) {
    return 0;
  }

  using entry_t = std::tuple<int, int, Node>;
  auto forward_unvisited = priority_queue<entry_t, std::greater<entry_t>>{};
  auto backward_unvisited = priority_queue<entry_t, std::greater<entry_t>>{};
  auto forward_distances = default_map<Node, int>{};
  auto backward_distances = default_map<Node, int>{};

  forward_distances.emplace(start_node, 0);
  backward_distances.emplace(end_node, 0);
  forward_unvisited.emplace(static_cast<int>(forward_heuristic(start_node)), 0,
                            start_node);
  backward_unvisited.emplace(static_cast<int>(backward_heuristic(end_node)), 0,
                             end_node);

  auto best_distance = std::optional<int>{};

  auto remove_stale = [](auto& unvisited, const auto& distances) {
    while (!unvisited.empty()) {
      const auto& [_, g_enqueued, current] = unvisited.top();
      if (distances.find(current)->second >= g_enqueued) {
        break;
      }
      unvisited.pop();
    }
  };

  auto search_direction = [&](auto& own_unvisited, auto& own_distances,
                              auto& other_distances, auto&& get_neighbors,
                              auto&& heuristic) {
    auto [_, g_enqueued, current] = own_unvisited.top();
    own_unvisited.pop();

    for (const auto& neighbor : get_neighbors(current)) {
      const int tentative_g = g_enqueued + neighbor.distance;
      auto [own_it, inserted] =
          own_distances.try_emplace(neighbor.node, tentative_g);
      if (!inserted && tentative_g >= own_it->second) {
        continue;
      }
      own_it->second = tentative_g;
      own_unvisited.emplace(
          tentative_g + static_cast<int>(heuristic(neighbor.node)), tentative_g,
          neighbor.node);

      if (const auto other_it = other_distances.find(neighbor.node);
          other_it != std::end(other_distances)) {
        const int total_distance = tentative_g + other_it->second;
        if (!best_distance || total_distance < *best_distance) {
          best_distance = total_distance;
        }
      }
    }
  };

  while (!forward_unvisited.empty() && !backward_unvisited.empty()) {
    remove_stale(forward_unvisited, forward_distances);
    remove_stale(backward_unvisited, backward_distances);
    if (forward_unvisited.empty() || backward_unvisited.empty()) {
      break;
    }

    const auto& [forward_f, _, __] = forward_unvisited.top();
    const auto& [backward_f, ___, ____] = backward_unvisited.top();
    if (best_distance &&
        (forward_f >= *best_distance) &&
        (backward_f >= *best_distance)) {
      break;
    }

    if (forward_f <= backward_f) {
      search_direction(forward_unvisited, forward_distances, backward_distances,
                       get_forward_neighbors, forward_heuristic);
    } else {
      search_direction(backward_unvisited, backward_distances,
                       forward_distances, get_backward_neighbors,
                       backward_heuristic);
    }
  }

  return best_distance;
}

template <std::totally_ordered Node>
class all_nodes_encountered {
 public:
  constexpr all_nodes_encountered(std::span<Node> nodes)
      : all_nodes_encountered{std::span<const Node>(nodes)} {}
  constexpr all_nodes_encountered(std::span<const Node> nodes)
      : m_searched_nodes{nodes | collect_vec<Node>()} {}

  constexpr bool operator()(Node node) {
    if (std::ranges::contains(m_searched_nodes, node)) {
      m_visited.insert(std::move(node));
      return m_visited.size() == m_searched_nodes.size();
    }
    return false;
  }

 private:
  std::vector<Node> m_searched_nodes;
  default_set<Node> m_visited;
};
template <has_value_type Container>
all_nodes_encountered(Container&&)
    -> all_nodes_encountered<typename Container::value_type>;

// Dijkstra convenience overloads (zero heuristic)

template <class ReturnT = void, class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}}, {},
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, Node&& end_node, NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      equal_to_value{std::forward<Node>(end_node)},
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, std::span<std::remove_cvref_t<Node>> end_nodes,
    NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      all_nodes_encountered{end_nodes},
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, std::span<const std::remove_cvref_t<Node>> end_nodes,
    NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      all_nodes_encountered{end_nodes},
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, EndReachedFn&& end_reached,
    NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<EndReachedFn>(end_reached),
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
  requires(!std::is_const_v<Node>)
constexpr auto shortest_distances_dijkstra(
    std::span<Node> start_nodes, EndReachedFn&& end_reached,
    NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{start_nodes},
      std::forward<EndReachedFn>(end_reached),
      std::forward<NeighborsFn>(get_reachable_neighbors), constant_value<int>{},
      predecessors_out);
}

template <class Node, class ForwardNeighborsFn, class BackwardNeighborsFn>
constexpr auto shortest_distance_bidirectional_dijkstra(
    Node&& start_node, Node&& end_node,
    ForwardNeighborsFn&& get_forward_neighbors,
    BackwardNeighborsFn&& get_backward_neighbors) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distance_bidirectional_astar(
      node_t{std::forward<Node>(start_node)},
      node_t{std::forward<Node>(end_node)},
      std::forward<ForwardNeighborsFn>(get_forward_neighbors),
      std::forward<BackwardNeighborsFn>(get_backward_neighbors));
}

// A* convenience overloads (caller-supplied heuristic)

template <class ReturnT = void, class Node, class NeighborsFn,
          class HeuristicFn, class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_astar(
    Node&& start_node, Node&& end_node, NeighborsFn&& get_reachable_neighbors,
    HeuristicFn&& heuristic, PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      equal_to_value{std::forward<Node>(end_node)},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      std::forward<HeuristicFn>(heuristic), predecessors_out);
}
template <class ReturnT = void, class Node, class NeighborsFn,
          class HeuristicFn, class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_astar(
    Node&& start_node, EndReachedFn&& end_reached,
    NeighborsFn&& get_reachable_neighbors, HeuristicFn&& heuristic,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<EndReachedFn>(end_reached),
      std::forward<NeighborsFn>(get_reachable_neighbors),
      std::forward<HeuristicFn>(heuristic), predecessors_out);
}

template <class ReturnT = void, class Node, class NeighborsFn,
          class HeuristicFn, class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
  requires(!std::is_const_v<Node>)
constexpr auto shortest_distances_astar(
    std::span<Node> start_nodes, EndReachedFn&& end_reached,
    NeighborsFn&& get_reachable_neighbors, HeuristicFn&& heuristic,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra<ReturnT>(
      std::span<const node_t>{start_nodes},
      std::forward<EndReachedFn>(end_reached),
      std::forward<NeighborsFn>(get_reachable_neighbors),
      std::forward<HeuristicFn>(heuristic), predecessors_out);
}

template <class Node, class ForwardNeighborsFn, class BackwardNeighborsFn,
          class ForwardHeuristicFn, class BackwardHeuristicFn>
constexpr auto shortest_distance_bidirectional_astar(
    Node&& start_node, Node&& end_node,
    ForwardNeighborsFn&& get_forward_neighbors,
    BackwardNeighborsFn&& get_backward_neighbors,
    ForwardHeuristicFn&& forward_heuristic,
    BackwardHeuristicFn&& backward_heuristic) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distance_bidirectional_astar(
      node_t{std::forward<Node>(start_node)},
      node_t{std::forward<Node>(end_node)},
      std::forward<ForwardNeighborsFn>(get_forward_neighbors),
      std::forward<BackwardNeighborsFn>(get_backward_neighbors),
      std::forward<ForwardHeuristicFn>(forward_heuristic),
      std::forward<BackwardHeuristicFn>(backward_heuristic));
}

template <class Node>
std::vector<Node> get_path(const predecessor_map<Node>& predecessors,
                           const Node& end_pos) {
  std::vector<Node> path;
  auto it = predecessors.find(end_pos);
  while (it != std::end(predecessors)) {
    path.push_back(it->first);
    it = predecessors.find(it->second);
  }
  return path;
}

/// Explores every state reachable from `start` via BFS.
/// get_neighbors expands a state into its adjacent states
/// (exclude invalid ones directly in that range,
/// e.g. by filtering out neighbors that fail some predicate).
/// Returns every state reachable from `start`, including `start` itself.
template <class ReturnT = void, class State, class NeighborsFn>
  requires requires(NeighborsFn get_neighbors, const State& state) {
    { get_neighbors(state) } -> std::ranges::input_range;
  }
constexpr auto flood_fill(State start, NeighborsFn&& get_neighbors) {
  auto visited = std::conditional_t<std::is_void_v<ReturnT>,
                                    std::unordered_set<State>, ReturnT>{};
  auto queue = std::deque<State>{};
  visited.insert(start);
  queue.push_back(std::move(start));
  while (!queue.empty()) {
    auto node = std::move(queue.front());
    queue.pop_front();
    for (auto&& neighbor : get_neighbors(node)) {
      // Standard sets report a fresh insertion through the pair's second,
      // bitmap_set returns the flag directly
      const bool inserted = [&] {
        if constexpr (std::same_as<decltype(visited.insert(neighbor)), bool>) {
          return visited.insert(neighbor);
        } else {
          return visited.insert(neighbor).second;
        }
      }();
      if (inserted) {
        queue.push_back(std::move(neighbor));
      }
    }
  }
  return visited;
}

/// Computes the longest distance from `start_node`
/// to every reachable node (the critical path).
/// The graph must be a DAG, cycles cause non-termination.
///
/// Derives a topological order via DFS post-order,
/// then relaxes edges in that order maximizing distances,
/// analogous to shortest_distances_dijkstra.
template <class ReturnT = void, class Node, class NeighborsFn>
  requires std::totally_ordered<Node>
constexpr auto longest_distances(Node start_node, NeighborsFn&& get_neighbors) {
  return longest_distances<ReturnT>(std::move(start_node),
                                    constant_value<bool>{},
                                    std::forward<NeighborsFn>(get_neighbors));
}

template <class ReturnT = void, class Node, class EndReachedFn,
          class NeighborsFn>
  requires std::totally_ordered<Node> && requires(Node node) {
    { std::declval<EndReachedFn>()(node) } -> std::convertible_to<bool>;
  }
constexpr auto longest_distances(Node start_node, EndReachedFn&& end_reached,
                                 NeighborsFn&& get_neighbors) {
  using distances_t = std::conditional_t<std::is_void_v<ReturnT>,
                                         default_map<Node, int>, ReturnT>;

  // Discover every reachable node via DFS,
  // recording each one when it finishes (post-order).
  // Reversing post-order gives a topological order:
  // for every edge u -> v, u comes before v.
  auto visited = default_set<Node>{};
  auto post_order = std::vector<Node>{};
  auto stack = std::vector<std::pair<Node, bool>>{};
  stack.emplace_back(start_node, false);
  visited.insert(start_node);

  while (!stack.empty()) {
    auto [node, finished] = stack.back();
    stack.pop_back();

    if (finished) {
      post_order.push_back(node);
      continue;
    }

    if (end_reached(node)) {
      post_order.push_back(node);
      continue;
    }

    stack.emplace_back(node, true);
    for (const auto& neighbor : get_neighbors(node)) {
      if (visited.insert(neighbor.node).second) {
        stack.emplace_back(neighbor.node, false);
      }
    }
  }

  // Relax edges in topological order.
  // By the time a node is processed,
  // its own longest distance from `start_node` is already final.
  auto distances = distances_t{};
  distances.emplace(start_node, 0);

  for (const auto& node : post_order | std::views::reverse) {
    auto it = distances.find(node);
    if (it == distances.end()) {
      continue;
    }
    const auto dist = it->second;

    for (const auto& neighbor : get_neighbors(node)) {
      const auto next_dist = dist + neighbor.distance;
      auto [ins_it, inserted] = distances.emplace(neighbor.node, next_dist);
      if (!inserted && next_dist > ins_it->second) {
        ins_it->second = next_dist;
      }
    }

    if (end_reached(node)) {
      break;
    }
  }

  return distances;
}

/// Generic depth-first search with memoization, using a caller-owned cache.
///
/// get_neighbors expands a state into a range of successor states, each
/// paired with a weight (dijkstra_neighbor_t::distance) -- a plain
/// dijkstra_uniform_neighbors_view() gives every edge weight 1, matching
/// a plain sum-of-neighbors search; a state-dependent leaf value can be
/// encoded as a single weighted edge to an already-end_reached state.
/// A state's value is 1 if end_reached(state), otherwise its neighbors'
/// (weight * value) terms folded together with `combine`, left to right
/// (Value{} if there are no neighbors, i.e. a dead end).
/// Returns the value computed for start_state; every state visited along
/// the way is also cached, so passing the same cache into further calls
/// (e.g. once per item in a batch) reuses that work instead of redoing it.
/// Call cache.clear() yourself between logically-independent searches.
template <class State, class EndReachedFn, class NeighborsFn, class CacheT,
          class CombineFn = std::plus<>>
  requires requires(EndReachedFn end_reached, NeighborsFn get_neighbors,
                    const State& state) {
    { end_reached(state) } -> std::convertible_to<bool>;
    { get_neighbors(state) } -> std::ranges::input_range;
  }
constexpr auto dfs(CacheT& cache, State start_state, EndReachedFn&& end_reached,
                   NeighborsFn&& get_neighbors, CombineFn&& combine = {}) {
  using Value = typename CacheT::mapped_type;
  const auto search = [&](this const auto& self_search,
                          State&& state) -> Value {
    if (const auto it = cache.find(state); it != cache.end()) {
      return it->second;
    }
    auto result =          //
        end_reached(state) //
            ? Value{1}
            : std::ranges::fold_left( //
                  get_neighbors(state) |
                      std::views::transform(
                          [&](dijkstra_neighbor_t<State> neighbor) -> Value {
                            return static_cast<Value>(neighbor.distance) *
                                   self_search(std::move(neighbor.node));
                          }),
                  Value{}, combine);
    cache.emplace(std::move(state), std::move(result));
    return result;
  };
  return search(std::move(start_state));
}

/// dfs that allocates its own cache (rather than reusing a caller-owned
/// one) and returns it in full, keyed by every state visited,
/// including start_state.
template <class ReturnT = void, class Value = std::uint64_t, class State,
          class EndReachedFn, class NeighborsFn, class CombineFn = std::plus<>>
  requires requires(EndReachedFn end_reached, NeighborsFn get_neighbors,
                    const State& state) {
    { end_reached(state) } -> std::convertible_to<bool>;
    { get_neighbors(state) } -> std::ranges::input_range;
  }
constexpr auto dfs(State start_state, EndReachedFn&& end_reached,
                   NeighborsFn&& get_neighbors, CombineFn&& combine = {}) {
  auto cache = std::conditional_t<std::is_void_v<ReturnT>,
                                  std::unordered_map<State, Value>, ReturnT>{};
  dfs(cache, std::move(start_state), std::forward<EndReachedFn>(end_reached),
      std::forward<NeighborsFn>(get_neighbors),
      std::forward<CombineFn>(combine));
  return cache;
}

/// dfs for the common case: every edge has weight 1
/// and the combine operator is addition.
/// A state's value is 1 if end_reached(state),
/// otherwise the sum of its neighbors' values (Value{} for a dead end).
template <class State, class EndReachedFn, class NeighborsFn, class CacheT>
  requires requires(EndReachedFn end_reached, NeighborsFn get_neighbors,
                    const State& state) {
    { end_reached(state) } -> std::convertible_to<bool>;
    { get_neighbors(state) } -> std::ranges::input_range;
  }
constexpr auto dfs_uniform(CacheT& cache, State start_state,
                           EndReachedFn&& end_reached,
                           NeighborsFn&& get_neighbors) {
  return dfs(cache, std::move(start_state),
             std::forward<EndReachedFn>(end_reached), [&](const State& state) {
               // as_rvalue lets States be moved rather than
               // copied out of get_neighbors' result, whether
               // it's an owned container or a lazy view This is
               // important since State may be expensive to copy
               return get_neighbors(state) |
                      std::views::as_rvalue |
                      dijkstra_uniform_neighbors_view();
             });
}

/// dfs_uniform that allocates its own cache -- see the two-argument dfs
/// for how this relates to the cache-taking overload above.
template <class ReturnT = void, class Value = std::uint64_t, class State,
          class EndReachedFn, class NeighborsFn>
  requires requires(EndReachedFn end_reached, NeighborsFn get_neighbors,
                    const State& state) {
    { end_reached(state) } -> std::convertible_to<bool>;
    { get_neighbors(state) } -> std::ranges::input_range;
  }
constexpr auto dfs_uniform(State start_state, EndReachedFn&& end_reached,
                           NeighborsFn&& get_neighbors) {
  auto cache = std::conditional_t<std::is_void_v<ReturnT>,
                                  std::unordered_map<State, Value>, ReturnT>{};
  dfs_uniform(cache, std::move(start_state),
              std::forward<EndReachedFn>(end_reached),
              std::forward<NeighborsFn>(get_neighbors));
  return cache;
}

} // AOC_EXPORT_NAMESPACE(aoc)

AOC_EXPORT_NAMESPACE(aoc) {
namespace ranges {

template <std::ranges::input_range R, class T, class Proj = std::identity>
  requires std::indirect_binary_predicate<
      std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>,
      const T*>
constexpr std::optional<std::size_t> position(R&& r, const T& value,
                                              Proj proj = {}) {
  auto it = std::ranges::find(r, value, std::move(proj));
  if (it == std::ranges::end(r)) {
    return std::nullopt;
  }
  return std::ranges::distance(std::ranges::begin(r), it);
}

} // namespace ranges
} // AOC_EXPORT_NAMESPACE(aoc)

template <class Node>
struct std::formatter<aoc::dijkstra_neighbor_t<Node>> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const aoc::dijkstra_neighbor_t<Node>& neighbor,
              std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{} ({})", neighbor.node,
                          neighbor.distance);
  }
};

#endif // AOC_ALGORITHM_H
