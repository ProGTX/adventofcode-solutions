#ifndef AOC_ALGORITHM_H
#define AOC_ALGORITHM_H

#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "dijkstra.h"
#include "flat.h"
#include "functional.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "string.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <deque>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

/// Swaps the rows and columns of a container of containers.
template <std::ranges::common_range Container>
constexpr Container transpose(const Container& container) {
  Container transposed_container;
  const auto num_rows = std::size(container);
  AOC_ASSERT(num_rows > 0, "Cannot transpose empty container");
  const auto num_columns = std::size(container[0]);
  AOC_ASSERT(num_columns > 0, "Cannot transpose empty container");
  transposed_container.resize(num_columns);
  for (unsigned row = 0; row < num_rows; ++row) {
    for (unsigned column = 0; column < num_columns; ++column) {
      transposed_container[column].resize(num_rows);
      transposed_container[column][row] = container[row][column];
    }
  }
  return transposed_container;
}
#if defined(AOC_COMPILER_MSVC)
static_assert(std::ranges::equal(transpose(std::vector{
                                     std::vector{1, 2, 3},
                                     std::vector{4, 5, 6},
                                     std::vector{7, 8, 9},
                                     std::vector{10, 11, 12},
                                 }),
                                 std::vector{
                                     std::vector{1, 4, 7, 10},
                                     std::vector{2, 5, 8, 11},
                                     std::vector{3, 6, 9, 12},
                                 }));
#endif
#if !defined(AOC_COMPILER_GCC)
static_assert(std::ranges::equal(transpose(std::vector<std::string>{
                                     "123",
                                     "456",
                                     "789",
                                     "ABC",
                                 }),
                                 std::vector<std::string>{
                                     "147A",
                                     "258B",
                                     "369C",
                                 }));
#endif
#if 0
// TODO
static_assert(std::ranges::equal(transpose(std::vector{
                                     std::array{1, 2, 3},
                                     std::array{4, 5, 6},
                                     std::array{7, 8, 9},
                                     std::array{10, 11, 12},
                                 }),
                                 std::vector{
                                     std::array{1, 4, 7, 10},
                                     std::array{2, 5, 8, 11},
                                     std::array{3, 6, 9, 12},
                                 }));
#endif

/// Pops the last element off a vector and returns it.
constexpr auto pop_stack(specialization_of<std::vector> auto&& container) {
  auto elem = std::move(container.back());
  container.resize(container.size() - 1);
  return elem;
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
  auto visited =
      std::conditional_t<std::is_void_v<ReturnT>, hash_set<State>, ReturnT>{};
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
constexpr auto critical_distances(Node start_node,
                                  NeighborsFn&& get_neighbors) {
  return critical_distances<ReturnT>(std::move(start_node),
                                     constant_value<bool>{},
                                     std::forward<NeighborsFn>(get_neighbors));
}

template <class ReturnT = void, class Node, class EndReachedFn,
          class NeighborsFn>
  requires std::totally_ordered<Node> && requires(Node node) {
    { std::declval<EndReachedFn>()(node) } -> std::convertible_to<bool>;
  }
constexpr auto critical_distances(Node start_node, EndReachedFn&& end_reached,
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

/// Longest simple path from `start` in a weighted graph
/// whose nodes are indexed 0..num_nodes.
///
/// get_neighbors expands a node into its successors,
/// each paired with an edge weight (dijkstra_neighbor_t::distance).
/// end_reached takes a node and how many nodes the path visits
/// including that one, so both "this is the exit"
/// and "every node has been visited" are expressible.
/// The search stops there, and returns the heaviest such path,
/// or nullopt if no path reaches one without revisiting a node.
///
/// No memoization: a node's value depends on which nodes the path already used.
/// Longest path is NP-hard once the graph has cycles,
/// so this is an exhaustive walk with backtracking.
template <class EndReachedFn, class NeighborsFn>
  requires requires(EndReachedFn end_reached, NeighborsFn get_neighbors,
                    std::size_t node, std::size_t num_visited) {
    { end_reached(node, num_visited) } -> std::convertible_to<bool>;
    { get_neighbors(node) } -> std::ranges::input_range;
  }
constexpr std::optional<int> longest_simple_path(std::size_t num_nodes,
                                                 std::size_t start,
                                                 EndReachedFn&& end_reached,
                                                 NeighborsFn&& get_neighbors) {
  // Not vector<bool>:
  // its packed proxy references cost a shift and a mask per access,
  // and none of that inlines in a Debug build.
  // The search probes this several times per edge,
  // which made it 3x the cost of the whole walk.
  auto visited = std::vector<char>(num_nodes);
  // The recursion passes a plain int rather than an optional,
  // with -1 for "no path from here".
  // Path weights are non-negative, so it cannot be mistaken for a real answer,
  // and the hot loop avoids constructing, comparing, and assigning
  // an optional per edge.
  constexpr auto no_path = -1;
  const auto search = [&](this const auto& self_search, const std::size_t node,
                          const std::size_t num_visited) -> int {
    if (end_reached(node, num_visited)) {
      return 0;
    }
    visited[node] = true;
    auto longest = no_path;
    for (const auto& neighbor : get_neighbors(node)) {
      if (visited[neighbor.node]) {
        continue;
      }
      const auto rest = self_search(neighbor.node, num_visited + 1);
      if (rest != no_path) {
        longest = std::max(longest, rest + neighbor.distance);
      }
    }
    visited[node] = false;
    return longest;
  };
  const auto longest = search(start, 1);
  return (longest != no_path) ? std::optional{longest} : std::nullopt;
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
                                  hash_map<State, Value>, ReturnT>{};
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
                                  hash_map<State, Value>, ReturnT>{};
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
