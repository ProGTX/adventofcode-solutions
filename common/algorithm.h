#ifndef AOC_ALGORITHM_H
#define AOC_ALGORITHM_H

#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "static_vector.h"
#include "string.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <array>
#include <compare>
#include <concepts>
#include <iostream>
#include <ranges>
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
    for (const auto& neighbor : get_reachable_neighbors(current)) {
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
  requires(!std::is_const_v<Node>)
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

template <std::integral counter_type = unsigned,
          std::ranges::sized_range ElementsR>
constexpr auto get_empty_combination(ElementsR&& elements,
                                     counter_type single_min = 0) {
  using underlying_type = decltype([&] {
    if constexpr (requires { elements.base(); }) {
      return elements.base();
    } else {
      return elements;
    };
  }());
  if constexpr (const auto N = max_container_elems<underlying_type>();
                N != std::string::npos) {
    return static_vector<counter_type, N>(std::ranges::size(elements),
                                          single_min);
  } else {
    return std::vector<counter_type>(std::ranges::size(elements), single_min);
  }
}

template <std::ranges::sized_range ElementsR, std::integral counter_type>
using combination_type =
    decltype(get_empty_combination<counter_type>(std::declval<ElementsR>()));

namespace ranges {

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
template <std::ranges::range ElementsR, std::integral counter_type = unsigned>
class combinations_view : public std::ranges::view_interface<
                              combinations_view<ElementsR, counter_type>> {
 public:
  using value_type =
      aoc::combination_type<decltype(std::declval<ElementsR>()), counter_type>;
  using reference = const value_type&;
  using const_reference = const value_type&;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;

 private:
  std::views::all_t<ElementsR> range;
  std::size_t num_elems;
  combinations_args<counter_type> args;

  class iterator {
   public:
    using value_type =
        aoc::combination_type<decltype(std::declval<ElementsR>()),
                              counter_type>;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

   private:
    const combinations_view* parent{nullptr};
    value_type combination;

    constexpr bool is_done() const { return combination.empty(); }

    constexpr bool is_valid() const {
      AOC_ASSERT(!this->is_done(),
                 "Can only call this function with an active iterator");
      const auto sum = ranges::accumulate(combination, counter_type{0});
      return (sum >= parent->args.all_min) && (sum <= parent->args.all_max);
    }

    constexpr void advance() {
      if (is_done()) {
        return;
      }

      const auto size = parent->num_elems;

      // Try to increment from the last position
      auto pos = size - 1;

      while (true) {
        if (combination[pos] < parent->args.single_max) {
          ++combination[pos];

          // Check if this path can lead to valid combinations
          const auto current_sum = ranges::accumulate(
              combination | std::views::take(pos + 1), counter_type{0});

          const auto remaining = static_cast<counter_type>(size - pos - 1);
          const auto max_possible =
              current_sum + remaining * parent->args.single_max;
          const auto min_possible =
              current_sum + remaining * parent->args.single_min;

          if ((max_possible >= parent->args.all_min) &&
              (min_possible <= parent->args.all_max)) {
            // Reset positions after current one to minimum
            std::ranges::fill(combination | std::views::drop(pos + 1),
                              parent->args.single_min);

            // Check if this combination is valid
            if (this->is_valid()) {
              return; // Found valid combination
            }

            // Otherwise continue searching
            pos = size - 1;
            continue;
          }
          // If pruned, continue incrementing
        } else if (pos == 0) {
          break;
        } else {
          // Backtrack
          pos--;
        }
      }

      combination.clear();
    }

    constexpr void find_first_valid() {
      // Check if initial combination is valid
      if (this->is_valid()) {
        return;
      }
      // Find first valid combination
      this->advance();
    }

   public:
    // Default constructor creates an end iterator
    constexpr iterator() noexcept = default;

    template <bool end>
    constexpr iterator(const combinations_view* parent, std::bool_constant<end>)
        : parent{parent}, combination{} {
      if constexpr (!end) {
        combination = aoc::get_empty_combination<counter_type>(
            parent->range, parent->args.single_min);
        this->find_first_valid();
      }
    }

    constexpr const auto& operator*() const { return combination; }
    constexpr const auto* operator->() const { return &combination; }

    constexpr iterator& operator++() {
      this->advance();
      return *this;
    }

    constexpr iterator operator++(int) {
      auto tmp = *this;
      this->advance();
      return tmp;
    }

    constexpr friend bool operator==(const iterator& lhs, const iterator& rhs) {
      return lhs.is_done() == rhs.is_done();
    }
    constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs) {
      return !(lhs == rhs);
    }
  };

 public:
  template <std::ranges::sized_range ElementsRDep = ElementsR>
  constexpr combinations_view(ElementsRDep&& range,
                              combinations_args<counter_type> args)
      : range{std::forward<ElementsRDep>(range)},
        num_elems{std::ranges::size(this->range)},
        args{args} {}

  constexpr iterator begin() const { return iterator{this, std::false_type{}}; }
  constexpr iterator end() const { return iterator{this, std::true_type{}}; }

  static_assert(std::sentinel_for<iterator, iterator>);
};
template <std::ranges::sized_range ElementsR,
          std::integral counter_type = unsigned>
combinations_view(ElementsR&&, combinations_args<counter_type>)
    -> combinations_view<std::views::all_t<ElementsR>, counter_type>;

} // namespace ranges

namespace detail {

template <std::integral counter_type>
struct counted_combinations_closure
    : std::ranges::range_adaptor_closure<
          counted_combinations_closure<counter_type>> {
  combinations_args<counter_type> args;

  template <std::ranges::range ElementsR>
  constexpr auto operator()(ElementsR&& elements) const {
    return ranges::combinations_view{std::forward<ElementsR>(elements), args};
  }
};
template <std::integral counter_type>
counted_combinations_closure(combinations_args<counter_type>)
    -> counted_combinations_closure<counter_type>;

struct counted_combinations_fn {
  template <std::integral counter_type = unsigned>
  constexpr auto operator()(combinations_args<counter_type> args) const {
    return counted_combinations_closure{.args = args};
  }

  template <std::ranges::range ElementsR, std::integral counter_type = unsigned>
  constexpr auto operator()(ElementsR&& elements,
                            combinations_args<counter_type> args) const {
    return counted_combinations_closure{.args = args}(
        std::forward<ElementsR>(elements));
  }
};

/// Corresponds to math combinations of (n k), where n is size(elements).
struct math_combinations_fn {
  template <std::integral counter_type>
  constexpr auto operator()(const counter_type k) const {
    return counted_combinations_closure{.args = combinations_args<counter_type>{
                                            .single_min = 0,
                                            .single_max = 1,
                                            .all_min = k,
                                            .all_max = k,
                                        }};
  }

  template <std::ranges::range ElementsR, std::integral counter_type>
  constexpr auto operator()(ElementsR&& elements, const counter_type k) const {
    return this->operator()(k)(std::forward<ElementsR>(elements));
  }
};

template <std::integral counter_type>
struct binary_combinations_fn
    : std::ranges::range_adaptor_closure<binary_combinations_fn<counter_type>> {
  constexpr auto operator()(counter_type num_elements) const {
    return counted_combinations_closure{.args = combinations_args<counter_type>{
                                            .single_min = 0,
                                            .single_max = 1,
                                            .all_min = 0,
                                            .all_max = num_elements,
                                        }};
  }

  template <std::ranges::range ElementsR>
  constexpr auto operator()(ElementsR&& elements) const {
    return this->operator()(static_cast<counter_type>(
        std::ranges::size(elements)))(std::forward<ElementsR>(elements));
  }
};

} // namespace detail

namespace views {

constexpr inline auto counted_combinations =
    aoc::detail::counted_combinations_fn{};

/// Corresponds to math combinations of (n k), where n is size(elements).
constexpr inline auto combinations = aoc::detail::math_combinations_fn{};

template <std::integral counter_type = unsigned>
constexpr inline auto binary_combinations =
    aoc::detail::binary_combinations_fn<counter_type>{};

} // namespace views

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
 * for (const auto& combo : counted_combinations(
 *   elements,
 *   combinations_args {
 *     .single_min = 0,
 *     .single_max = 1,
 *     .all_min = 0,
 *     .all_max = 2,
 * ) {
 *     const auto selected = binary_select_from_combination(
 *       elements, combo);
 *     // `selected` contains references to the original elements
 *     assert(selected.size() <= 2);
 *   }
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

static_assert(std::movable<aoc::ranges::combinations_view<std::vector<int>>>);

static_assert(
    std::ranges::view<aoc::ranges::combinations_view<std::vector<int>>>);
static_assert(
    std::ranges::range<aoc::ranges::combinations_view<std::vector<int>>>);

#endif // AOC_ALGORITHM_H
