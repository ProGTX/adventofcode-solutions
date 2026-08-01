#ifndef AOC_PARSE_H
#define AOC_PARSE_H

#include "assert.h"
#include "compiler.h"

#ifndef AOC_MODULE_SUPPORT
#include <charconv>
#include <concepts>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

template <class value_type = int>
constexpr auto to_number_with_rest(std::string_view str, int base = 10) {
  AOC_ASSERT(str.size() > 0, "Cannot parse empty string");
  if (str[0] == '+') {
    str = str.substr(1);
  }
  auto first = str.data();
  auto last = first + str.size();
  value_type value;
  auto result = std::from_chars_result{};
  if constexpr (std::integral<value_type>) {
    result = std::from_chars(first, last, value, base);
  } else if constexpr (std::floating_point<value_type>) {
    // TODO: Support other formats
    result = std::from_chars(first, last, value, std::chars_format::general);
  } else {
    static_assert(false, "Parsing not supported for this type");
  }
  if (result.ec != std::errc{}) [[unlikely]] {
    throw std::runtime_error("to_number failed to parse " + std::string(str));
  }
  return std::pair<value_type, std::string_view>{
      value, str.substr(std::distance(&str[0], result.ptr))};
}

template <class value_type = int>
constexpr auto to_number(std::string_view str, int base = 10) {
  return to_number_with_rest<value_type>(str, base).first;
}
template <class value_type = int>
constexpr auto to_number(char c) {
  return static_cast<value_type>(c - '0');
}
template <class value_type = int>
constexpr auto to_number(char c, int base) {
  return to_number<value_type>(std::string_view{&c, 1}, base);
}

template <class T>
struct number_converter {
  int base = 10;

  constexpr number_converter() = default;
  constexpr number_converter(int base) : base{base} {}

  constexpr T operator()(std::string_view str) const {
    return to_number<T>(str, base);
  }
  constexpr T operator()(char c) const {
    //
    return to_number<T>(c, base);
  }
  template <std::integral U = T>
  constexpr U operator()(std::integral auto value) const {
    return static_cast<U>(value);
  }
};

/**
 * Converts a range representing a binary number to a single decimal number.
 *
 * @see string.h for string specializations
 */
template <auto one = 1, class return_t = unsigned, std::ranges::range R,
          bool force_this = false>
constexpr return_t binary_to_number(const R& range) {
  auto num = return_t{};
  auto multiplier = return_t{1};
  for (const auto& v : range) {
    num += multiplier * static_cast<return_t>(v == one);
    multiplier *= 2;
  }
  return num;
}

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_PARSE_H
