#ifndef AOC_PRINT_H
#define AOC_PRINT_H

#include "compiler.h"
#include "concepts.h"
#include "ratio.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <concepts>
#include <iostream>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

// TODO: Should be replaced by C++23 std::format

template <class R>
struct print_range {
  R range;
  std::string_view separator;
  bool braces;

  explicit constexpr print_range(R range_, std::string_view separator_ = ",",
                                 bool braces_ = true)
      : range{std::move(range_)}, separator{separator_}, braces{braces_} {}

  constexpr friend std::ostream& operator<<(std::ostream& out,
                                            const print_range& printer) {
    if constexpr (contains_type<std::decay_t<R>, std::string,
                                std::string_view>) {
      out << printer.range;
    } else if constexpr (std::ranges::range<R>) {
      if (printer.braces) {
        out << '{';
      }
      for (const auto& item : printer.range) {
        out << print_range<decltype(item)>{item} << printer.separator;
      }
      if (printer.braces) {
        out << '}';
      }
    } else if constexpr (is_specialization_of_v<std::decay_t<R>, std::pair>) {
      out << '(' << printer.range.first << ',' << printer.range.second << ')';
    } else {
      out << printer.range;
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
printable_tuple(Types...) -> printable_tuple<Types...>;

namespace aoc {
template <class T>
std::ostream& operator<<(std::ostream& out, const ratio<T>& value) {
  out << '(' << value.numerator() << '/' << value.denominator() << ')';
  return out;
}
} // namespace aoc

constexpr void println(std::string_view str = "") {
  AOC_NOT_CONSTEXPR(std::cout << str << std::endl);
}

template <class FirstT, class... Args>
constexpr void println(std::string_view str, FirstT first, Args&&... args) {
  AOC_NOT_CONSTEXPR({
    std::cout << str << ": " << std::forward<FirstT>(first);
    // https://stackoverflow.com/a/27375675
    ((std::cout << ',' << std::forward<Args>(args)), ...);
    std::cout << std::endl;
  });
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_PRINT_H
