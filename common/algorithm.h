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
#include <array>
#include <compare>
#include <concepts>
#include <iostream>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

// Specifies a neighbor of the current node
// along with the cost (distance) of reaching it from the current node
template <class Node>
struct dijkstra_neighbor_t {
  Node node;
  int distance;

  constexpr bool operator==(const dijkstra_neighbor_t&) const = default;
  constexpr auto operator<=>(const dijkstra_neighbor_t&) const = default;

  friend std::ostream& operator<<(std::ostream& out,
                                  const dijkstra_neighbor_t& neighbor) {
    out << neighbor.node << " (" << neighbor.distance << ")";
    return out;
  }
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

template <class Node>
using predecessor_map =
    flat_map<std::remove_cvref_t<Node>, std::remove_cvref_t<Node>>;

template <class Node>
using predecessor_map_all =
    flat_map<std::remove_cvref_t<Node>, flat_set<std::remove_cvref_t<Node>>>;

// https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#Algorithm
// NOTE: We basically use a priority queue, but use the flat_set for that,
//       because std::priority_queue is still not constexpr in C++23.
//       That's also why we sort by std::greater instead of std::less.
template <class Node, class NeighborsFn,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
  requires std::totally_ordered<Node> &&
           requires(Node node) {
             {
               *std::begin(std::declval<NeighborsFn>()(node))
             } -> std::convertible_to<dijkstra_neighbor_t<Node>>;
             { std::declval<EndReachedFn>()(node) } -> std::same_as<bool>;
           } &&
           contains_uncvref<PredecessorMap, predecessor_map<Node>,
                            predecessor_map_all<Node>>
constexpr flat_map<Node, int> shortest_distances_dijkstra(
    std::span<const Node> start_nodes, NeighborsFn&& get_reachable_neighbors,
    EndReachedFn&& end_reached = {},
    PredecessorMap* predecessors_out = nullptr) {
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
    if (end_reached(current)) {
      break;
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
          if constexpr (!all_predecessors) {
            if (use_predecessors) {
              (*predecessors_out)[neighbor.node] = current;
            }
          }
        }
        if constexpr (all_predecessors) {
          if (use_predecessors && (new_neighbor_dist <= neighbor_dist)) {
            (*predecessors_out)[neighbor.node].emplace(current);
          }
        }
      } else {
        // Neighbor hasn't been visited yet
        distances.try_emplace(neighbor.node, new_neighbor_dist);
        unvisited.emplace(neighbor.node);
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

template <std::totally_ordered Node>
class all_nodes_encountered {
 public:
  constexpr all_nodes_encountered(std::span<Node> nodes)
      : all_nodes_encountered{std::span<const Node>(nodes)} {}
  constexpr all_nodes_encountered(std::span<const Node> nodes)
      : m_searched_nodes{nodes | ranges::to<std::vector<Node>>()} {}

  constexpr bool operator()(Node node) {
    if (std::ranges::contains(m_searched_nodes, node)) {
      m_visited.insert(std::move(node));
      return m_visited.size() == m_searched_nodes.size();
    }
    return false;
  }

 private:
  std::vector<Node> m_searched_nodes;
  flat_set<Node> m_visited;
};
template <has_value_type Container>
all_nodes_encountered(Container&&)
    -> all_nodes_encountered<typename Container::value_type>;

template <class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors), {}, predecessors_out);
}
template <class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors, Node&& end_node,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      equal_to_value{std::forward<Node>(end_node)}, predecessors_out);
}
template <class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    std::span<std::remove_cvref_t<Node>> end_nodes,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      all_nodes_encountered{end_nodes}, predecessors_out);
}
template <class Node, class NeighborsFn,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    std::span<const std::remove_cvref_t<Node>> end_nodes,
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      all_nodes_encountered{end_nodes}, predecessors_out);
}
template <class Node, class NeighborsFn,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    Node&& start_node, NeighborsFn&& get_reachable_neighbors,
    EndReachedFn&& end_reached, PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{std::array{std::forward<Node>(start_node)}},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      std::forward<EndReachedFn>(end_reached), predecessors_out);
}
template <class Node, class NeighborsFn,
          class EndReachedFn = constant_value<bool>,
          class PredecessorMap = predecessor_map<Node>>
constexpr auto shortest_distances_dijkstra(
    std::span<Node> start_nodes, NeighborsFn&& get_reachable_neighbors,
    EndReachedFn&& end_reached = {},
    PredecessorMap* predecessors_out = nullptr) {
  using node_t = std::remove_cvref_t<Node>;
  return shortest_distances_dijkstra(
      std::span<const node_t>{start_nodes},
      std::forward<NeighborsFn>(get_reachable_neighbors),
      std::forward<EndReachedFn>(end_reached), predecessors_out);
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

template <std::integral counter_type>
struct combinations_args {
  counter_type single_min;
  counter_type single_max;
  counter_type all_min;
  counter_type all_max;
};

namespace detail {

template <std::integral counter_type, std::ranges::sized_range Combination,
          class CallbackFn, class EarlyReturnFn>
constexpr void gen_combinations_recursive(
    Combination& combination, CallbackFn& callback, EarlyReturnFn& early_return,
    const combinations_args<counter_type>& args, const std::size_t pos,
    const std::size_t size, const counter_type current_sum) {
  // Base case: we've filled all positions
  if (pos == size) {
    if ((current_sum >= args.all_min) && (current_sum <= args.all_max)) {
      callback(static_cast<const Combination&>(combination));
    }
    return;
  }

  // Calculate how many positions are left to fill
  const auto remaining_positions = size - pos;

  // Try each possible value for the current position
  for (auto val = args.single_min; val <= args.single_max; ++val) {
    if (early_return()) {
      break;
    }
    const auto new_sum = current_sum + val;

    // Prune: check if it's still possible to reach all_min
    // (even if we fill remaining positions with single_max)
    const auto max_possible_sum =
        new_sum + (remaining_positions - 1) * args.single_max;
    if (max_possible_sum < args.all_min) {
      continue; // Skip this branch
    }

    // Prune: check if we've already exceeded all_max
    // (even if we fill remaining positions with single_min)
    const auto min_possible_sum =
        new_sum + (remaining_positions - 1) * args.single_min;
    if (min_possible_sum > args.all_max) {
      break; // No need to try larger values
    }

    combination[pos] = val;
    gen_combinations_recursive<counter_type>(
        combination, callback, early_return, args, pos + 1, size, new_sum);
  }
}

} // namespace detail

/**
 * Iterates over all combinations of size N, with limitations.
 *
 * - N is the size of elements.
 * - A combination is std::array if N is known at compile time,
 *   std::vector otherwise.
 * - Each element ranges from args.single_min to args.single_max (inclusive).
 * - Sum of all elements is between args.all_min and args.all_max (inclusive).
 * - Only generates valid combinations.
 * - Calls the callback on each valid combination.
 */
template <std::integral counter_type = unsigned,
          std::ranges::sized_range ElementsR, class CallbackFn,
          class EarlyReturnFn = std::false_type>
  requires std::is_invocable_r_v<bool, EarlyReturnFn>
constexpr void gen_combinations(ElementsR&& elements,
                                const combinations_args<counter_type> args,
                                CallbackFn&& callback,
                                EarlyReturnFn&& early_return = {}) {
  if ((args.single_max < args.single_min) ||
      (args.all_max < args.all_min) ||
      (args.single_max > args.all_max)) {
    return;
  }
  auto combination = [&] {
    if constexpr (const auto N = max_container_elems<ElementsR>();
                  N != std::string::npos) {
      return std::array<counter_type, N>{};
    } else {
      return std::vector<counter_type>(std::ranges::size(elements), 0);
    }
  }();
  const auto size = combination.size();
  if (size == 0) {
    return;
  }
  if ((size * args.single_min) > args.all_max) {
    throw std::runtime_error("Element-wise minimum exceeds global maximum");
  }
  constexpr std::size_t pos = 0;
  constexpr counter_type current_sum = 0;
  detail::gen_combinations_recursive<counter_type>(
      combination, callback, early_return, args, pos, size, current_sum);
}

template <std::integral counter_type = unsigned, std::ranges::sized_range R,
          class CallbackFn, class EarlyReturnFn = std::false_type>
constexpr void binary_combinations(R&& elements, CallbackFn&& callback,
                                   EarlyReturnFn&& early_return = {}) {
  // No need to forward elements and callback, can be used as references
  return gen_combinations<counter_type>(
      elements,
      combinations_args<counter_type>{
          .single_min = 0,
          .single_max = 1,
          .all_min = 0,
          .all_max = static_cast<counter_type>(std::ranges::size(elements)),
      },
      callback, early_return);
}

/// Corresponds to math combinations of (n k), where n is size(elements).
template <std::integral counter_type = unsigned, std::ranges::sized_range R,
          class CallbackFn, class EarlyReturnFn = std::false_type>
constexpr void simple_combinations(R&& elements, const counter_type k,
                                   CallbackFn&& callback,
                                   EarlyReturnFn&& early_return = {}) {
  // No need to forward elements and callback, can be used as references
  return gen_combinations<counter_type>(elements,
                                        combinations_args<counter_type>{
                                            .single_min = 0,
                                            .single_max = 1,
                                            .all_min = k,
                                            .all_max = k,
                                        },
                                        callback, early_return);
}

/**
 * Given a combination configuration of elements, returns a list of elements
 * for which the combination indicates their selection.
 * Used in conjunction with gen_combinations or binary_combinations.
 *
 * The default return type is a vector of references to the elements,
 * can be changed by specifying output_t.
 * Return type can be anything that inserter_it supports.
 *
 * Mostly useful when single_min is (0 or 1) and single_max is 1.
 * Can be used outside of those parameters, but the amount information
 * is lost in this function, so the interpretation is up to the user.
 *
 * all_min and all_max can be used as normal.
 *
 * @code
 * const auto elements = std::array{1, 2, 3, 4, 5};
 * gen_combinations(
 *   elements,
 *   combinations_args {
 *     .single_min = 0,
 *     .single_max = 1,
 *     .all_min = 0,
 *     .all_max = 2,
 *   },
 *   [&](auto&& combo) {
 *     const auto selected = binary_select_from_combination(
 *       elements, combo);
 *     // `selected` contains references to the original elements
 *     assert(selected.size() <= 2);
 *   }
 * );
 * @endcode
 */
template <class output_t = void, std::ranges::sized_range ElementsR,
          std::ranges::sized_range CombinationR>
constexpr auto binary_select_from_combination(ElementsR&& elements,
                                              CombinationR&& combination) {
  using element_type =
      std::remove_reference_t<decltype(*std::ranges::begin(elements))>;
  auto output = [] {
    if constexpr (std::same_as<output_t, void>) {
      return std::vector<std::reference_wrapper<element_type>>{};
    } else {
      return output_t{};
    }
  }();
  constexpr const auto store_element = [] {
    if constexpr (std::same_as<output_t, void>) {
      return [](auto&& value) { return std::ref(value); };
    } else {
      return std::identity{};
    }
  }();
  const auto end = insertion_end_it(output);
  for (auto inserter = inserter_it(output);
       auto&& [element, combo] :
       std::views::zip(std::forward<ElementsR>(elements),
                       std::forward<CombinationR>(combination))) {
    if (combo) {
      *inserter = store_element(element);
      ++inserter;
    }
    if constexpr (std::same_as<decltype(end), std::unreachable_sentinel_t>) {
      // No check for end
    } else if (inserter == end) {
      break;
    }
  }
  return output;
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_ALGORITHM_H
