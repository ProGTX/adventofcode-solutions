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

template <std::ranges::input_range View, class F>
  requires std::ranges::view<View> &&
           std::invocable<F&, std::ranges::range_reference_t<View>> &&
           specialization_of<
               std::invoke_result_t<F&, std::ranges::range_reference_t<View>>,
               std::optional>
class transform_filter_view
    : public std::ranges::view_interface<transform_filter_view<View, F>> {
  using mapped_t = typename std::invoke_result_t<
      F&, std::ranges::range_reference_t<View>>::value_type;

  View base{};
  [[no_unique_address]] F user_function{};

 public:
  constexpr transform_filter_view()
    requires std::default_initializable<View> && std::default_initializable<F>
  = default;

  constexpr transform_filter_view(View base, F user_function)
      : base{std::move(base)}, user_function{std::move(user_function)} {}

  class iterator {
    transform_filter_view* parent = nullptr;
    std::ranges::iterator_t<View> current{};
    std::optional<mapped_t> value_opt{};

    /// Advances the current value until function returns an engaged optional
    /// (caching the result), or until it reaches the end.
    constexpr void advance_to_match() {
      const auto last = std::ranges::end(parent->base);
      for (; current != last; ++current) {
        value_opt = std::invoke(parent->user_function, *current);
        if (value_opt.has_value()) {
          return;
        }
      }
      value_opt.reset();
    }

   public:
    using value_type = mapped_t;
    using difference_type = std::ranges::range_difference_t<View>;
    using iterator_concept = std::ranges::iterator_t<std::ranges::filter_view<
        View, ::aoc::constant_value<bool>>>::iterator_concept;

    iterator() = default;
    constexpr iterator(transform_filter_view* parent,
                       std::ranges::iterator_t<View> current)
        : parent{parent}, current{std::move(current)} {
      advance_to_match();
    }

    constexpr const mapped_t& operator*() const { return *value_opt; }

    constexpr iterator& operator++() {
      ++current;
      advance_to_match();
      return *this;
    }
    constexpr iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    // Precondition: a matching element exists before `current`,
    // i.e. this iterator is not `begin()`.
    constexpr iterator& operator--()
      requires std::ranges::bidirectional_range<View>
    {
      do {
        --current;
        value_opt = std::invoke(parent->user_function, *current);
      } while (!value_opt.has_value());
      return *this;
    }
    constexpr iterator operator--(int)
      requires std::ranges::bidirectional_range<View>
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr bool operator==(iterator const& other) const {
      return current == other.current;
    }
    constexpr bool operator==(std::default_sentinel_t) const {
      return !value_opt.has_value();
    }
  };

  constexpr iterator begin() {
    return iterator{this, std::ranges::begin(base)};
  }
  constexpr std::default_sentinel_t end() { return {}; }
};
template <class R, class F>
transform_filter_view(R&&, F) -> transform_filter_view<std::views::all_t<R>, F>;

} // namespace ranges

namespace detail {

template <class F>
struct transform_filter_closure
    : std::ranges::range_adaptor_closure<transform_filter_closure<F>> {
  [[no_unique_address]] F fun;

  constexpr explicit transform_filter_closure(F fun) : fun{std::move(fun)} {}

  template <std::ranges::viewable_range R>
  constexpr auto operator()(R&& r) const {
    return ::aoc::ranges::transform_filter_view{std::forward<R>(r), fun};
  }
};
template <class F>
transform_filter_closure(F) -> transform_filter_closure<F>;

} // namespace detail

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

/// Performs a filter and a transform in a single view.
///
/// Similar to Rust `filter_map`,
/// filters out based on whether the returned optional is engaged or not.
/// Resulting values are automatically unwrapped.
/// The provided function is invoked at most once per element.
///
/// Name chosed based on this:
/// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2760r0.html
template <class F>
constexpr auto transform_filter(F&& f) {
  return ::aoc::detail::transform_filter_closure{std::forward<F>(f)};
}
static_assert(std::ranges::equal(
    std::views::iota(0, 6) | transform_filter([](int i) -> std::optional<int> {
      return (i % 2 == 0) ? std::optional{i * 10} : std::nullopt;
    }),
    std::array{0, 20, 40}));

} // namespace views

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_RANGES_H
