#pragma once

#include <array>
#include <iterator>
#include <type_traits>

template <class T>
struct inspect_t;

template <auto V>
struct inspect_v;

// https://stackoverflow.com/a/51032862
// https://open-std.org/JTC1/SC22/WG21/docs/papers/2020/p2098r1.pdf
template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type {};
template <template <class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};
template <class T, template <class...> class Primary>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<T, Primary>::value;

template <class T, template <class...> class Primary>
concept specialization_of = is_specialization_of_v<T, Primary>;

template <class Container>
concept insertable =
    requires(Container c, typename Container::value_type value) {
      c.begin();
      c.end();
      c.insert(value);
    };

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

static_assert(contains_type<std::string_view, std::string_view>);
static_assert(contains_type<std::string_view, int, std::string_view>);
