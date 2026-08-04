#ifndef AOC_COMBINATIONS_H
#define AOC_COMBINATIONS_H

#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "ranges.h"
#include "static_vector.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <concepts>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>
#endif
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
  // N is deliberately not an if-constexpr init-statement:
  // a variable declared there has no linkage,
  // and GCC 15 then treats `static_vector<counter_type, N>`
  // as a TU-local entity that this exported function must not expose.
  constexpr auto N = static_size<underlying_type>();
  if constexpr (N != std::string::npos) {
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
      const auto& args = parent->args;

      // Try to increment from the last position
      auto pos = size - 1;

      while (true) {
        // Everything before pos is fixed,
        // and the positions after it can still contribute anything in
        // `[remaining * single_min, remaining * single_max]`.
        // So the sums reachable through a value v at pos
        // form a contiguous window that slides with v,
        // which pins the feasible v to a contiguous range -
        // stepping one at a time only to re-test the same inequality
        // is what made this walk `0..single_max` on every dead end.
        const auto before =
            static_cast<counter_type>(prefix_sum - combination[pos]);
        const auto remaining = static_cast<counter_type>(size - pos - 1);
        const auto floor_sum =
            static_cast<counter_type>(before + remaining * args.single_min);
        const auto ceil_sum =
            static_cast<counter_type>(before + remaining * args.single_max);

        // v must reach all_min even with every later position maxed out
        const auto lo =
            (args.all_min <= ceil_sum)
                ? args.single_min
                : std::max(args.single_min,
                           static_cast<counter_type>(args.all_min - ceil_sum));
        // and stay under all_max even with every later position at its minimum
        const auto hi =
            (args.all_max < floor_sum)
                ? args.single_min // empty range, hi < lo forces a backtrack
                : std::min(args.single_max,
                           static_cast<counter_type>(args.all_max - floor_sum));
        const auto next =
            std::max(static_cast<counter_type>(combination[pos] + 1), lo);

        if ((combination[pos] < args.single_max) &&
            (next <= hi) &&
            (args.all_max >= floor_sum)) {
          combination[pos] = next;

          // Reset positions after current one to minimum
          std::ranges::fill(combination | std::views::drop(pos + 1),
                            args.single_min);
          const auto total_sum = static_cast<counter_type>(floor_sum + next);
          prefix_sum = total_sum;

          // hi already caps the sum at all_max, so only the floor can fail,
          // and it takes larger values further right to lift it
          if (total_sum >= args.all_min) {
            return; // Found valid combination
          }
          pos = size - 1;
          continue;
        }

        if (pos == 0) {
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

/**
 * Enumerates every subset of a range as a bitmask, dereferencing yields
 * a static_vector of pointers to the selected elements directly
 * (unlike combinations_view, which yields per-element counts).
 * Limited to at most (bits in mask_type - 1) elements,
 * since a full bitmask needs one more bit to represent "one past the end".
 *
 * NOTE: combinations_args{.single_min = 0, .single_max = 1,
 *                         .all_min = 0, .all_max = num_elements}
 *       provides essentially the same functionality, but it's much slower.
 *
 * NOTE: We have to return pointers instead of references (or wrappers)
 *       because `static_vector` requires types to be default constructible,
 *       which can't be easily changed in C++23.
 *       This needs to be revisited with C++26 and inplace_vector.
 */
template <std::ranges::forward_range R,
          std::unsigned_integral mask_type = std::uint32_t>
  requires std::ranges::sized_range<R>
class binary_combinations_view : public std::ranges::view_interface<
                                     binary_combinations_view<R, mask_type>> {
  using element_type = std::ranges::range_value_t<R>;
  static_assert(std::is_lvalue_reference_v<std::ranges::range_reference_t<R>>,
                "binary_combinations_view needs a range of addressable "
                "elements -- dereferencing an iterator must yield a real "
                "reference, not a generated/temporary value");

  static constexpr std::size_t max_bits =
      std::numeric_limits<mask_type>::digits;
  static constexpr std::size_t static_elems = aoc::static_size<R>();

  // A subset can never reach max_bits elements:
  // `mask_type{1} << max_bits` would be UB
  // (needed for "one past the last mask")
  static constexpr std::size_t capacity =
      (static_elems == std::string::npos) ? max_bits : static_elems;
  static_assert(static_elems == std::string::npos || static_elems < max_bits,
                "binary_combinations_view supports at most "
                "(bits in mask_type - 1) elements");

 public:
  using selection_type = aoc::static_vector<const element_type*, capacity>;

 private:
  R* range;

  // Custom iterator allows us to support forward_range
  class iterator {
    R* range = nullptr;
    mask_type mask = 0;
    selection_type current;

    constexpr void materialize() {
      current.clear();
      mask_type i = 0;
      for (auto&& elem : *range) {
        if (mask & (mask_type{1} << i)) {
          current.emplace_back(std::addressof(elem));
        }
        ++i;
      }
    }

   public:
    using value_type = selection_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    iterator() = default;
    constexpr iterator(R* range, mask_type mask) : range{range}, mask{mask} {
      materialize();
    }

    constexpr const selection_type& operator*() const { return current; }

    constexpr iterator& operator++() {
      ++mask;
      materialize();
      return *this;
    }
    constexpr iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr friend bool operator==(const iterator& lhs, const iterator& rhs) {
      return lhs.mask == rhs.mask;
    }
  };

 public:
  constexpr explicit binary_combinations_view(R& r) : range{&r} {
    // Compile-time-sized ranges are already covered by the static_assert above
    // Only the dynamic case needs a runtime check
    if constexpr (static_elems == std::string::npos) {
      AOC_ASSERT(std::ranges::size(r) < max_bits,
                 "binary_combinations_view supports at most "
                 "(bits in mask_type - 1) elements");
    }
  }

  constexpr iterator begin() const { return iterator{range, mask_type{0}}; }
  constexpr iterator end() const {
    return iterator{range, static_cast<mask_type>(
                               mask_type{1} << std::ranges::size(*range))};
  }
};
template <class R, std::unsigned_integral mask_type = std::uint32_t>
binary_combinations_view(R&) -> binary_combinations_view<R, mask_type>;

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

template <std::unsigned_integral mask_type>
struct binary_combinations_fn
    : std::ranges::range_adaptor_closure<binary_combinations_fn<mask_type>> {
  template <std::ranges::viewable_range ElementsR>
  constexpr auto operator()(ElementsR&& elements) const {
    return ranges::binary_combinations_view<std::remove_reference_t<ElementsR>,
                                            mask_type>{elements};
  }
};

} // namespace detail

namespace views {

constexpr inline auto counted_combinations =
    aoc::detail::counted_combinations_fn{};

/// Corresponds to math combinations of (n k), where n is size(elements).
constexpr inline auto combinations = aoc::detail::math_combinations_fn{};

/// Enumerates every subset of a range,
/// yielding pointers to the selected elements directly
template <std::unsigned_integral mask_type = std::uint32_t>
constexpr auto binary_combinations() {
  return aoc::detail::binary_combinations_fn<mask_type>{};
}
template <std::unsigned_integral mask_type = std::uint32_t,
          std::ranges::viewable_range ElementsR>
constexpr auto binary_combinations(ElementsR&& elements) {
  return aoc::detail::binary_combinations_fn<mask_type>{}(
      std::forward<ElementsR>(elements));
}

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
