#pragma once

#include <concepts>
#include <iostream>
#include <ranges>
#include <string_view>
#include <tuple>

#include "common.h"
#include "utility.h"

template <std::ranges::range R>
struct print_range {
  R range;
  std::string_view separator;

  explicit constexpr print_range(R range_, std::string_view separator_ = ",")
      : range{std::move(range_)}, separator{separator_} {}

  constexpr friend std::ostream& operator<<(std::ostream& out,
                                            const print_range& printer) {
    for (const auto& item : printer.range) {
      if constexpr (std::ranges::range<decltype(item)>) {
        out << '{' << print_range<decltype(item)>{item} << '}'
            << printer.separator;
      } else {
        out << item << printer.separator;
      }
    }

    return out;
  }
};

// https://stackoverflow.com/a/67687348/793006
template <std::size_t I, class... Ts>
void print_tuple(std::ostream& out, const std::tuple<Ts...>& tuple) {
  if constexpr (I == sizeof...(Ts)) {
    out << ')';
  } else {
    using current_t = decltype(std::get<I>(tuple));
    if constexpr (std::ranges::range<current_t> &&
                  !std::convertible_to<current_t, std::string>) {
      out << '{' << print_range(std::get<I>(tuple)) << '}';
    } else {
      out << std::get<I>(tuple);
    }
    if constexpr (I + 1 != sizeof...(Ts)) {
      out << ",";
    }
    print_tuple<I + 1>(out, tuple);
  }
}

template <class... Types>
struct printable_tuple : public std::tuple<Types...> {
 private:
  using base_t = std::tuple<Types...>;

 public:
  using base_t::base_t;

  friend std::ostream& operator<<(std::ostream& out,
                                  const printable_tuple& tuple) {
    out << '(';
    print_tuple<0>(out, tuple);
    return out;
  }
};
template <class... Types>
printable_tuple(Types...)->printable_tuple<Types...>;

template <class T>
std::ostream& operator<<(std::ostream& out, const fractional_t<T>& value) {
  out << static_cast<std::conditional_t<(sizeof(T) > 4), double, float>>(value);
  return out;
}
