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

template <std::ranges::forward_range... Rs>
constexpr auto dot_product(Rs&&... rs) {
  auto zipped = std::views::zip(rs...);
  using value_type = std::iter_value_t<std::ranges::iterator_t<
      std::remove_reference_t<std::tuple_element_t<0, std::tuple<Rs...>>>>>;
  return std::ranges::fold_left(
      zipped, value_type{}, [](auto acc, auto&& tuple) {
        auto product =
            std::apply([](auto&&... elems) { return (elems * ...); }, tuple);
        return acc + product;
      });
}
static_assert(270 == dot_product(std::array{1, 2, 3}, //
                                 std::array{4, 5, 6}, //
                                 std::array{7, 8, 9}  //
                                 ));

// Shouldn't be needed once all compilers support extending containers
// using ranges (__cpp_lib_containers_ranges)
template <class T, std::ranges::input_range R>
void extend(std::vector<T>& vec, R&& r) {
  if constexpr (std::ranges::sized_range<R>) {
    vec.insert(std::end(vec), std::ranges::begin(r), std::ranges::end(r));
  } else {
    for (auto&& elem : r) {
      vec.push_back(std::forward<decltype(elem)>(elem));
    }
  }
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
