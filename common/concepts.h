#ifndef AOC_CONCEPTS_H
#define AOC_CONCEPTS_H

#include "compiler.h"
#include "range_to.h"

#ifndef AOC_MODULE_SUPPORT
#include <array>
#include <iterator>
#include <string_view>
#include <type_traits>
#endif

template <class T>
struct inspect_t;

template <auto V>
struct inspect_v;

AOC_EXPORT_NAMESPACE(aoc) {

// https://stackoverflow.com/a/51032862
// https://open-std.org/JTC1/SC22/WG21/docs/papers/2020/p2098r1.pdf
template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type {};
template <template <class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};
template <class T, template <class...> class Primary>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<std::remove_cvref_t<T>, Primary>::value;

template <template <class...> class Primary, class... Args>
using reuse_primary_t = Primary<Args...>;
template <template <class, auto, class...> class Primary, class T, auto Size,
          class... Args>
using reuse_primary_with_size_t = Primary<T, Size, Args...>;

template <class T, template <class...> class Primary>
concept specialization_of = is_specialization_of_v<T, Primary>;

template <class Container>
concept insertable = ranges::detail::insertable_container<Container>;

template <class Container>
concept back_insertable =
    requires(Container c, typename Container::value_type value) {
      c.begin();
      c.end();
      c.push_back(value);
    };

template <class Container>
concept has_value_type =
    requires(Container c) { typename Container::value_type; };

template <class Container>
concept reservable = ranges::detail::reservable_container<Container>;

template <class T>
struct is_array_class : std::false_type {};
template <class T, std::size_t N>
struct is_array_class<std::array<T, N>> : std::true_type {};
template <class T>
inline constexpr bool is_array_class_v = is_array_class<T>::value;

template <class output_t>
constexpr auto inserter_it(output_t& elems) {
  if constexpr (insertable<output_t>) {
    return std::inserter(elems, std::end(elems));
  } else if constexpr (back_insertable<output_t>) {
    return std::back_inserter(elems);
  } else {
    return std::begin(elems);
  }
}

// https://stackoverflow.com/a/68066873/793006
template <typename Op, typename Arg1, typename Arg2>
concept binary_op = std::is_invocable_v<Op, Arg1, Arg2>;
template <typename Op, typename Ret, typename Arg1, typename Arg2>
concept binary_op_r = std::is_invocable_r_v<Ret, Op, Arg1, Arg2>;

template <typename T, typename... U>
concept contains_type = (std::same_as<T, U> || ...);

template <typename T, typename... U>
concept contains_uncvref = (std::same_as<std::remove_cvref_t<T>, U> || ...);

static_assert(contains_type<std::string_view, std::string_view>);
static_assert(contains_type<std::string_view, int, std::string_view>);

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_CONCEPTS_H
