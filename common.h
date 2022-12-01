#pragma once

#include <concepts>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// https://stackoverflow.com/a/51032862
template <class, template <class...> class>
inline constexpr bool is_specialization = false;
template <template <class...> class T, class... Args>
inline constexpr bool is_specialization<T<Args...>, T> = true;

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
  for (int linenum = 0; std::getline(file, line); ++linenum) {
    if constexpr (std::invocable<OpT, std::string_view, int>) {
      operation(trim(line), linenum);
    } else {
      operation(trim(line));
    }
  }
  file.close();
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
template <typename OutItT>
void split_line_to_iterator(const std::string& input, char delimiter,
                            OutItT outputIt) {
  std::stringstream stream{input};
  for (std::string item; std::getline(stream, item, delimiter); ++outputIt) {
    *outputIt = std::move(item);
  }
}

template <class OutputT>
OutputT split(const std::string& input, char delimiter) {
  OutputT elems;
  auto outputIt = std::begin(elems);
  if constexpr (is_specialization<OutputT, std::vector>) {
    outputIt = std::back_insert_iterator(elems);
  }
  split_line_to_iterator(input, delimiter, outputIt);
  return elems;
}
