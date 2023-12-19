#pragma once

#include "range_to.h"

#include <iterator>
#include <numeric>
#include <ranges>

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
