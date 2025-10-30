#ifndef AOC_RANGES_H
#define AOC_RANGES_H

#include "compiler.h"
#include "range_to.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

namespace ranges {

struct __accumulate_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T>
  constexpr auto operator()(I first, S last, T init) const {
    return std::ranges::fold_left(std::move(first), std::move(last),
                                  std::move(init), std::plus<>{});
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
    return std::ranges::fold_left(
        std::move(first), std::move(last), T(1),
        [](T&& first, T&& second) { return std::lcm(first, second); });
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
    return std::ranges::fold_left(
        std::move(first), std::move(last), T(1),
        [](T&& first, T&& second) { return std::gcd(first, second); });
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

template <std::permutable I, std::sentinel_for<I> S>
constexpr auto rotate_left(I first, S last) {
  return std::ranges::rotate(first, first + 1, last);
}
template <std::ranges::forward_range R>
  requires std::permutable<std::ranges::iterator_t<R>>
constexpr auto rotate_left(R&& r) {
  return rotate_left(std::ranges::begin(r), std::ranges::end(r));
}

template <std::permutable I, std::sentinel_for<I> S>
constexpr auto rotate_right(I first, S last) {
  return std::ranges::rotate(std::move(first), std::move(last) - 1, last);
}
template <std::ranges::forward_range R>
  requires std::permutable<std::ranges::iterator_t<R>>
constexpr auto rotate_right(R&& r) {
  return rotate_right(std::ranges::begin(r), std::ranges::end(r));
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

} // namespace views

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_RANGES_H
