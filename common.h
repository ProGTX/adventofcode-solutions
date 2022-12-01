#pragma once

#include <concepts>
#include <fstream>
#include <string>
#include <string_view>

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

template <std::invocable<std::string_view, int> OpT>
void readfile_op(const std::string& filename, OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 0; std::getline(file, line); ++linenum) {
    operation(trim(line), linenum);
  }
  file.close();
}
