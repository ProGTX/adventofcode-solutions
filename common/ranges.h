#ifndef AOC_RANGES_H
#define AOC_RANGES_H

#include "compiler.h"
#include "range_to.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#endif

AOC_EXPORT namespace aoc {

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
  return std::ranges::rotate(first, last - 1, last);
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

// Very rough approximation of std::views::join_with_view
// https://en.cppreference.com/w/cpp/ranges/join_with_view
template <std::ranges::viewable_range R, class Pattern>
constexpr std::string join_with(R&& r, Pattern&& pattern) {
  std::string result;
  int num_elems = 0;
  for (auto&& value : r | std::views::join) {
    result.push_back(static_cast<char>(value));
    if constexpr (std::same_as<Pattern, char>) {
      result.push_back(pattern);
    } else {
      for (auto&& p : pattern) {
        result.push_back(p);
      }
    }
    ++num_elems;
  }
  if (num_elems > 0) {
    const int pattern_size = [&]() -> std::size_t {
      if constexpr (std::same_as<Pattern, char>) {
        return 1;
      } else {
        return std::size(pattern);
      }
    }();
    result.resize(result.size() - pattern_size);
  }
  return result;
}
template <std::ranges::viewable_range R>
constexpr std::string join_with(R&& r, const char* pattern) {
  return join_with(std::forward<R>(r), std::string_view(pattern));
}

} // namespace views

} // namespace aoc

#endif // AOC_RANGES_H
