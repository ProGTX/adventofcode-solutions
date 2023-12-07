#pragma once

#include <charconv>
#include <concepts>
#include <exception>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

// https://stackoverflow.com/a/63050738/793006
constexpr std::string_view ltrim(std::string_view str,
                                 std::string_view whitespace) {
  const auto pos(str.find_first_not_of(whitespace));
  str.remove_prefix(std::min(pos, str.length()));
  return str;
}
constexpr std::string_view rtrim(std::string_view str,
                                 std::string_view whitespace) {
  const auto pos(str.find_last_not_of(whitespace));
  str.remove_suffix(std::min(str.length() - pos - 1, str.length()));
  return str;
}
constexpr std::string_view trim(std::string_view str,
                                std::string_view whitespace = " \t\n\r\f\v") {
  str = ltrim(str, whitespace);
  str = rtrim(str, whitespace);
  return str;
}
constexpr std::string_view trim_simple(std::string_view str) {
  return trim(str);
}

template <class return_t = std::string_view>
constexpr auto get_trimmer() {
  return [](std::string_view str) { return return_t{trim(str)}; };
};
template <class return_t = std::string_view>
constexpr auto get_trimmer_keep_spaces() {
  return [](std::string_view str) { return return_t{trim(str, "\t\n\r\f\v")}; };
};

template <class value_type>
constexpr auto to_number(std::string_view str) {
  auto first = str.data();
  auto last = first + str.size();
  value_type value;
  auto result = std::from_chars(first, last, value);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("Failed to parse " + std::string(result.ptr));
  }
  return value;
}

template <class trim_op_t = decltype(get_trimmer()), class OpT>
  requires(std::invocable<OpT, std::string_view, int> ||
           std::invocable<OpT, std::string_view>)
void readfile_op(const std::string& filename, OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 1; std::getline(file, line); ++linenum) {
    if constexpr (std::invocable<OpT, std::string_view, int>) {
      operation(trim_op_t{}(line), linenum);
    } else {
      operation(trim_op_t{}(line));
    }
  }
  file.close();
}

template <class FirstLineOpT, class OpT>
  requires(std::invocable<FirstLineOpT, std::string_view> &&
           std::invocable<OpT, std::string_view>)
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

std::vector<std::string> readfile_lines(const std::string& filename) {
  std::vector<std::string> lines;
  readfile_op<decltype(get_trimmer_keep_spaces())>(
      filename,
      [&](std::string_view line) { lines.push_back(std::string{line}); });
  return lines;
}

std::vector<int> readfile_numbers(const std::string& filename) {
  std::vector<int> numbers;
  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    int currentVal = to_number<int>(line);
    numbers.push_back(currentVal);
  });
  return numbers;
}

// https://stackoverflow.com/a/236803
template <size_t max_elements = std::string::npos, class out_it_t,
          class item_op_t = std::identity>
void split_line_to_iterator(std::string_view input, char delimiter,
                            out_it_t outputIt, item_op_t item_op = {}) {
  std::stringstream stream{std::string{input}};
  size_t index = 0;
  for (std::string item; std::getline(stream, item, delimiter);) {
    if (item.empty()) {
      continue;
    }
    *outputIt = item_op(std::move(item));

    // Must increase as last step, in case an item was skipped
    ++outputIt;
    if constexpr (max_elements < std::string::npos) {
      ++index;
      if (index >= max_elements) {
        break;
      }
    }
  }
}

template <class output_t, class string_item_op_t = std::identity>
constexpr auto split_item_op() {
  if constexpr (has_value_type<output_t>) {
    using value_type = typename output_t::value_type;
    if constexpr (std::integral<value_type>) {
      return [](auto&& item) {
        return to_number<value_type>(string_item_op_t{}(item));
      };
    } else {
      return string_item_op_t{};
    }
  } else {
    return string_item_op_t{};
  }
}

template <class output_t>
constexpr size_t max_container_elems() {
  if constexpr (is_array_class_v<output_t>) {
    return std::tuple_size<output_t>::value;
  } else {
    return std::string::npos;
  }
}

template <class output_t, class string_item_op_t = std::identity>
output_t split(std::string_view input, char delimiter) {
  output_t elems;
  split_line_to_iterator<max_container_elems<output_t>()>(
      input, delimiter, inserter_it(elems),
      split_item_op<output_t, string_item_op_t>());
  return elems;
}
