#ifndef AOC_COMBINATIONS_H
#define AOC_COMBINATIONS_H

#include "assert.h"
#include "compiler.h"
#include "ranges.h"
#include "static_vector.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <concepts>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

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
  if constexpr (const auto N = static_size<underlying_type>();
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

    // Sum of combination[0..=pos],
    // where pos is advance()'s current search position.
    // Maintained incrementally (a handful of +/- per step)
    // instead of recomputed from scratch via accumulate on every single
    // increment attempt, which is what made this view expensive to use in a
    // hot search loop: this is the same combination either way,
    // just found without redoing the same partial sums over and over.
    counter_type prefix_sum{};

    constexpr bool is_done() const { return combination.empty(); }

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
          ++prefix_sum;

          // Check if this path can lead to valid combinations
          const auto remaining = static_cast<counter_type>(size - pos - 1);
          const auto max_possible =
              prefix_sum + remaining * parent->args.single_max;
          const auto min_possible =
              prefix_sum + remaining * parent->args.single_min;

          if ((max_possible >= parent->args.all_min) &&
              (min_possible <= parent->args.all_max)) {
            // Reset positions after current one to minimum
            std::ranges::fill(combination | std::views::drop(pos + 1),
                              parent->args.single_min);
            const auto total_sum =
                prefix_sum + remaining * parent->args.single_min;

            if ((total_sum >= parent->args.all_min) &&
                (total_sum <= parent->args.all_max)) {
              prefix_sum = total_sum;
              return; // Found valid combination
            }

            // Otherwise continue searching
            prefix_sum = total_sum;
            pos = size - 1;
            continue;
          }
          // If pruned, continue incrementing
        } else if (pos == 0) {
          break;
        } else {
          // Backtrack
          prefix_sum -= combination[pos];
          pos--;
        }
      }

      combination.clear();
    }

    constexpr void find_first_valid() {
      // One-time full sum to seed the incremental total tracked from here on
      prefix_sum = ranges::accumulate(combination, counter_type{0});
      // Check if initial combination is valid
      if ((prefix_sum >= parent->args.all_min) &&
          (prefix_sum <= parent->args.all_max)) {
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

#endif // AOC_COMBINATIONS_H
