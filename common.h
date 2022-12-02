#pragma once

#include <array>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// https://stackoverflow.com/a/51032862
template <class, template <class...> class>
inline constexpr bool is_specialization = false;
template <template <class...> class T, class... Args>
inline constexpr bool is_specialization<T<Args...>, T> = true;

template <class T>
struct is_array_class : std::false_type {};
template <class T, std::size_t N>
struct is_array_class<std::array<T, N>> : std::true_type {};
template <class T>
inline constexpr bool is_array_class_v = is_array_class<T>::value;

// https://stackoverflow.com/a/63050738/793006
constexpr std::string_view ltrim(std::string_view str) {
  const auto pos(str.find_first_not_of(" \t\n\r\f\v"));
  str.remove_prefix(std::min(pos, str.length()));
  return str;
}
constexpr std::string_view rtrim(std::string_view str) {
  const auto pos(str.find_last_not_of(" \t\n\r\f\v"));
  str.remove_suffix(std::min(str.length() - pos - 1, str.length()));
  return str;
}
constexpr std::string_view trim(std::string_view str) {
  str = ltrim(str);
  str = rtrim(str);
  return str;
}

template <class OpT>
requires std::invocable<OpT, std::string_view, int> ||
    std::invocable<OpT, std::string_view>
void readfile_op(const std::string& filename, OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 1; std::getline(file, line); ++linenum) {
    if constexpr (std::invocable<OpT, std::string_view, int>) {
      operation(trim(line), linenum);
    } else {
      operation(trim(line));
    }
  }
  file.close();
}

template <class FirstLineOpT, class OpT>
requires std::invocable<FirstLineOpT, std::string_view> &&
    std::invocable<OpT, std::string_view>
void readfile_op_header(const std::string& filename,
                        FirstLineOpT first_line_operation, OpT operation) {
  readfile_op(filename, [&](std::string_view line, int linenum) {
    if (linenum == 1) {
      first_line_operation(line);
    } else {
      operation(line);
    }
  });
}

std::vector<int> readfile_numbers(const std::string& filename) {
  std::vector<int> numbers;
  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    int currentVal = std::stoi(std::string{line});
    numbers.push_back(currentVal);
  });
  return numbers;
}

// https://stackoverflow.com/a/236803
template <class out_it_t, class item_op_t = std::identity>
void split_line_to_iterator(const std::string& input, char delimiter,
                            out_it_t outputIt, item_op_t item_op = {}) {
  std::stringstream stream{input};
  for (std::string item; std::getline(stream, item, delimiter);) {
    if (item.empty()) {
      continue;
    }
    *outputIt = item_op(std::move(item));
    // Must increase as last step, in case an item was skipped
    ++outputIt;
  }
}

template <class output_t>
constexpr auto split_container_it(output_t& elems) {
  if constexpr (is_array_class_v<output_t>) {
    return std::begin(elems);
  } else {
    return std::back_insert_iterator(elems);
  }
}

template <class output_t>
constexpr auto split_item_op() {
  if constexpr (is_array_class_v<output_t> ||
                is_specialization<output_t, std::vector>) {
    if constexpr (std::is_same_v<typename output_t::value_type, int>) {
      return [](auto&& item) { return std::stoi(item); };
    } else {
      return std::identity{};
    }
  } else {
    return std::identity{};
  }
}

template <class output_t>
output_t split(const std::string& input, char delimiter) {
  output_t elems;
  split_line_to_iterator(input, delimiter, split_container_it(elems),
                         split_item_op<output_t>());
  return elems;
}

// https://en.cppreference.com/w/cpp/utility/to_underlying
template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}
