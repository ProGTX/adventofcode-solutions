#ifndef AOC_ALGORITHM_H
#define AOC_ALGORITHM_H

#include "compiler.h"
#include "concepts.h"
#include "dijkstra.h"
#include "flat.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "string.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <concepts>
#include <deque>
#include <functional>
#include <optional>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

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

#endif // AOC_ALGORITHM_H
