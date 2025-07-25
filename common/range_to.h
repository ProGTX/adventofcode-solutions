#ifndef AOC_RANGE_TO_H
#define AOC_RANGE_TO_H

/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License license. See LICENSE.md for details.

https://github.com/cor3ntin/ranges/blob/master/include/cor3ntin/ranges/to.hpp

*/

#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>
#endif

AOC_EXPORT namespace aoc {

namespace ranges {

struct from_range_t {};
inline constexpr from_range_t from_range;

namespace detail {

namespace r = std::ranges;

template <typename T>
inline constexpr bool always_false_v = false;

template <r::range Rng>
struct range_common_iterator_impl {
  using type =
      std::common_iterator<std::ranges::iterator_t<Rng>, r::sentinel_t<Rng>>;
};

template <typename Rng>
struct dummy_input_iterator {
  using iterator_category = std::input_iterator_tag;
  using value_type = std::ranges::range_value_t<Rng>;
  using difference_type = std::ranges::range_difference_t<Rng>;
  ;
  using pointer = std::ranges::range_value_t<Rng>*;
  using reference = std::ranges::range_reference_t<Rng>;

  int operator*() const;
  bool operator==(const dummy_input_iterator& other) const;
  reference operator++(int);
  dummy_input_iterator& operator++();
};

template <r::common_range Rng>
struct range_common_iterator_impl<Rng> {
  using type = r::iterator_t<Rng>;
};

template <r::range Rng>
  requires(!std::copyable<std::ranges::iterator_t<Rng>>)
struct range_common_iterator_impl<Rng> {
  using type = dummy_input_iterator<Rng>;
};

template <r::range Rng>
using range_common_iterator = typename range_common_iterator_impl<Rng>::type;

template <typename C>
struct container_value;

template <typename C>
  requires requires { typename r::range_value_t<C>; }
struct container_value<C> {
  using type = r::range_value_t<C>;
};
template <typename C>
  requires(!requires { typename r::range_value_t<C>; })
struct container_value<C> {
  using type = typename C::value_type;
};

template <typename C>
using container_value_t = typename container_value<C>::type;

template <class C, class R>
concept container_convertible =
    !r::view<C> && r::input_range<R> &&
    std::convertible_to<r::range_reference_t<R>, container_value_t<C>>;

template <class C, class R>
concept recursive_container_convertible =
    container_convertible<C, R> ||
    (r::input_range<r::range_reference_t<R>> && requires {
      {
        to<r::range_value_t<C>>(std::declval<r::range_reference_t<R>>())
      } -> std::convertible_to<r::range_value_t<C>>;
    });

} // namespace detail

template <typename Cont, std::ranges::input_range Rng, typename... Args>
  requires detail::recursive_container_convertible<Cont, Rng>
constexpr auto to(Rng&& rng, Args&&... args) -> Cont;

namespace detail {

template <template <class...> class T>
struct wrap {};

template <typename Cont, typename Rng, typename... Args>
struct unwrap {
  using type = Cont;
};

template <template <class...> class Cont, typename Rng, typename... Args>
struct unwrap<wrap<Cont>, Rng, Args...> {
  template <typename R>
  static auto from_rng(int)
      -> decltype(Cont(range_common_iterator<Rng>(),
                       range_common_iterator<Rng>(), std::declval<Args>()...));
  template <typename R>
  static auto from_rng(long) -> decltype(Cont(from_range, std::declval<Rng>(),
                                              std::declval<Args>()...));

  using type =
      std::remove_cvref_t<std::remove_pointer_t<decltype(from_rng<Rng>(0))>>;
};

template <typename T>
concept reservable_container = requires(T& c) {
  c.reserve(r::size(c));
  { c.capacity() } -> std::same_as<decltype(r::size(c))>;
  { c.max_size() } -> std::same_as<decltype(r::size(c))>;
};

template <typename T>
concept insertable_container =
    requires(T& c, typename T::value_type& e) { c.insert(c.end(), e); };

struct to_container {
 private:
  template <typename ToContainer, typename Rng, typename... Args>
  using container_t = typename unwrap<ToContainer, Rng, Args...>::type;

 public:
  template <typename C, typename... Args>
  struct fn {
   private:
    template <typename Cont, typename Rng>
    constexpr static auto construct(Rng&& rng, Args&&... args) {
      auto inserter = [](Cont& c) {
        if constexpr (requires {
                        c.push_back(std::declval<
                                    std::ranges::range_reference_t<Rng>>());
                      }) {
          return std::back_inserter(c);
        } else {
          return std::inserter(c, std::end(c));
        }
      };

      // copy or move (optimization)
      if constexpr (std::constructible_from<Cont, Rng, Args...>) {
        return Cont(std::forward<Rng>(rng), std::forward<Args>(args)...);
      } else if constexpr (std::constructible_from<Cont, from_range_t, Rng,
                                                   Args...>) {
        return Cont(from_range, std::forward<Rng>(rng),
                    std::forward<Args>(args)...);
      } else if constexpr (r::common_range<Rng> &&
                           std::constructible_from<Cont, r::iterator_t<Rng>,
                                                   r::iterator_t<Rng>,
                                                   Args...>) {
        return Cont(r::begin(rng), r::end(rng), std::forward<Args>(args)...);
      }
      // we can do push back
      else if constexpr (insertable_container<Cont> &&
                         std::constructible_from<Cont, Args...>) {
        Cont c(std::forward<Args>(args)...);
        if constexpr (r::sized_range<Rng> && reservable_container<Cont>) {
          c.reserve(r::size(rng));
        }
        r::copy(std::forward<Rng>(rng), inserter(c));
        return c;
      } else {
        static_assert(always_false_v<Cont>, "Can't construct a container");
      }
    }

   private:
    template <typename Cont, r::range Rng>
      requires container_convertible<Cont, Rng>
    constexpr static auto impl(Rng&& rng, Args&&... args) {
      return construct<Cont>(std::forward<Rng>(rng),
                             std::forward<Args>(args)...);
    }

    template <typename Cont, r::range Rng>
      requires recursive_container_convertible<Cont, Rng> &&
               std::constructible_from<Cont, Args...> &&
               (!container_convertible<Cont, Rng> &&
                !std::constructible_from<Cont, Rng>)
    constexpr static auto impl(Rng&& rng, Args&&... args) {

      return to<Cont, Args...>(rng | r::views::transform([](auto&& elem) {
                                 return to<r::range_value_t<Cont>>(
                                     std::forward<decltype(elem)>(elem));
                               }),
                               std::forward<Args>(args)...);
    }

   public:
    template <typename Rng>
      requires r::input_range<Rng> &&
               recursive_container_convertible<container_t<C, Rng, Args...>,
                                               Rng&&>
    inline constexpr auto operator()(Rng&& rng, Args&&... args) const {
      return impl<container_t<C, Rng, Args...>>(std::forward<Rng>(rng),
                                                std::forward<Args>(args)...);
    }
    std::tuple<Args...> args;
  };

  template <typename Rng, typename ToContainer, typename... Args>
    requires r::input_range<Rng> &&
             recursive_container_convertible<
                 container_t<ToContainer, Rng, Args...>, Rng>
  constexpr friend auto operator|(Rng&& rng, fn<ToContainer, Args...>&& f)
      -> container_t<ToContainer, Rng, Args...> {
    return [&]<size_t... I>(std::index_sequence<I...>) {
      return f(std::forward<Rng>(rng),
               std::forward<Args>(std::get<I>(f.args))...);
    }(std::make_index_sequence<sizeof...(Args)>());
  }
};

template <typename ToContainer, typename... Args>
using to_container_fn = to_container::fn<ToContainer, Args...>;
} // namespace detail

template <template <typename...> class ContT, typename... Args,
          auto val = detail::to_container{}>
  requires(!std::ranges::range<Args> && ...)
constexpr auto to(Args&&... args)
    -> detail::to_container_fn<detail::wrap<ContT>, Args...> {
  detail::to_container_fn<detail::wrap<ContT>, Args...> fn;
  fn.args = std::forward_as_tuple(std::forward<Args>(args)...);
  return fn;
}

template <template <typename...> class ContT, std::ranges::input_range Rng,
          typename... Args>
  requires std::ranges::range<Rng>
constexpr auto to(Rng&& rng, Args&&... args) {
  return detail::to_container_fn<detail::wrap<ContT>, Args...>{}(
      std::forward<Rng>(rng), std::forward<Args>(args)...);
}

template <typename Cont, typename... Args, auto val = detail::to_container{}>
  requires(!std::ranges::range<Args> && ...)
constexpr auto to(Args&&... args) -> detail::to_container_fn<Cont, Args...> {
  detail::to_container_fn<Cont, Args...> fn;
  fn.args = std::forward_as_tuple(std::forward<Args>(args)...);
  return fn;
}

template <typename Cont, std::ranges::input_range Rng, typename... Args>
  requires detail::recursive_container_convertible<Cont, Rng>
constexpr auto to(Rng&& rng, Args&&... args) -> Cont {
  return detail::to_container_fn<Cont, Args...>{}(std::forward<Rng>(rng),
                                                  std::forward<Args>(args)...);
}

} // namespace ranges

} // namespace aoc

#endif // AOC_RANGE_TO_H
