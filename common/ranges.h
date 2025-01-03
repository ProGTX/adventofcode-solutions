#ifndef AOC_RANGES_H
#define AOC_RANGES_H

#include "compiler.h"
#include "range_to.h"
#include "utility.h"

#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>

namespace aoc {

namespace ranges {

// https://en.cppreference.com/w/cpp/algorithm/ranges/contains
struct __contains_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T,
            class Proj = std::identity>
    requires std::indirect_binary_predicate<std::ranges::equal_to,
                                            std::projected<I, Proj>, const T*>
  constexpr bool operator()(I first, S last, const T& value,
                            Proj proj = {}) const {
    return std::ranges::find(std::move(first), last, value, proj) != last;
  }

  template <std::ranges::input_range R, class T, class Proj = std::identity>
    requires std::indirect_binary_predicate<
        std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>,
        const T*>
  constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(value),
                   proj);
  }
};
inline constexpr __contains_fn contains{};

// https://en.cppreference.com/w/cpp/algorithm/ranges/fold_left
struct __fold_left_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T, class F>
  constexpr auto operator()(I first, S last, T init, F f) const {
    using U =
        std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>;
    if (first == last) {
      return U(std::move(init));
    }
    U accum = std::invoke(f, std::move(init), *first);
    for (++first; first != last; ++first) {
      accum = std::invoke(f, std::move(accum), *first);
    }
    return std::move(accum);
  }

  template <std::ranges::input_range R, class T, class F>
  constexpr auto operator()(R&& r, T init, F f) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(init),
                   std::ref(f));
  }
};
inline constexpr __fold_left_fn fold_left;

struct __accumulate_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T>
  constexpr auto operator()(I first, S last, T init) const {
    return fold_left(first, last, std::move(init), std::plus<>{});
  }

  template <std::ranges::input_range R, class T>
  constexpr auto operator()(R&& r, T init) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(init));
  }
};
inline constexpr __accumulate_fn accumulate;

struct __lcm_fn {
  template <std::input_iterator I, std::sentinel_for<I> S>
  constexpr auto operator()(I first, S last) const {
    using T = std::iter_value_t<I>;
    return fold_left(first, last, T(1),
                     [](T first, T second) { return std::lcm(first, second); });
  }

  template <std::ranges::input_range R>
  constexpr auto operator()(R&& r) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r));
  }
};
inline constexpr __lcm_fn lcm;

struct gcd_fn {
  template <std::input_iterator I, std::sentinel_for<I> S>
  constexpr auto operator()(I first, S last) const {
    using T = std::iter_value_t<I>;
    return fold_left(first, last, T(1),
                     [](T first, T second) { return std::gcd(first, second); });
  }

  template <std::ranges::input_range R>
  constexpr auto operator()(R&& r) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r));
  }
};
inline constexpr gcd_fn gcd;

template <std::ranges::random_access_range R, class Comp = std::ranges::less,
          class Proj = std::identity>
constexpr auto sorted(R&& r, Comp comp = {}, Proj proj = {}) {
  auto r_copy = r;
  std::ranges::sort(r_copy, comp, proj);
  return r_copy;
}

} // namespace ranges

namespace views {

template <class W>
constexpr auto transform_to_value(W&& value) {
  return std::views::transform(
      ::aoc::constant_value<W>{std::forward<W>(value)});
}

template <class return_t>
constexpr auto transform_cast() {
  return std::views::transform(::aoc::transform_cast<return_t>{});
}

template <class T>
constexpr auto to_number(int base = 10) {
  return std::views::transform(::aoc::number_converter<T>{base});
}

// Not quite std::views::repeat, but close enough
// https://en.cppreference.com/w/cpp/ranges/repeat_view
template <class W>
constexpr auto repeat(W&& value) {
  return std::views::iota(0) | transform_to_value(std::forward<W>(value));
}
template <class W, class Bound>
  requires(std::integral<std::remove_cvref_t<Bound>>)
constexpr auto repeat(W&& value, Bound&& bound) {
  using bound_t = std::remove_cvref_t<Bound>;
  return std::views::iota(static_cast<bound_t>(0), std::forward<Bound>(bound)) |
         transform_to_value(std::forward<W>(value));
}
static_assert(std::ranges::equal(std::array{7, 7, 7}, repeat(7, 3)));
#if !defined(AOC_COMPILER_CLANG)
static_assert(std::ranges::equal(std::array{"ha", "ha"},
                                 repeat("ha") | std::views::take(2)));
#endif

// Approximate implementation of std::views::stride
// https://en.cppreference.com/w/cpp/ranges/stride_view
template <class DifferenceType>
constexpr auto stride(DifferenceType&& n) {
  return std::views::filter(
      [index = DifferenceType{0},
       n = std::forward<DifferenceType>(n)](auto) mutable {
        if (index == 0) {
          ++index;
          return true;
        }
        ++index;
        if (index == n) {
          index = 0;
        }
        return false;
      });
}
static_assert(std::ranges::equal(std::array{1, 4, 7, 10},
                                 std::views::iota(1, 13) | stride(3)));
static_assert(std::ranges::equal(std::array{10, 7, 4, 1},
                                 std::views::iota(1, 13) | stride(3) |
                                     std::views::reverse));
static_assert(std::ranges::equal(std::array{12, 9, 6, 3},
                                 std::views::iota(1, 13) | std::views::reverse |
                                     stride(3)));

// Approximate implementation of std::views::enumerate
// https://en.cppreference.com/w/cpp/ranges/enumerate_view
// NOTE: Deviations from C++23:
//   * Overload with no arguments should be an object instance, not a function
constexpr auto enumerate() {
  return std::views::transform([index = 0]<class T>(T&& val) mutable {
    return std::pair<int, decltype(val)>{index++, std::forward<T>(val)};
  });
}
template <std::ranges::viewable_range R>
constexpr auto enumerate(R&& r) {
  return r | enumerate();
}

} // namespace views

} // namespace aoc

#endif // AOC_RANGES_H
