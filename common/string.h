#pragma once

#include <charconv>
#include <concepts>
#include <exception>
#include <fstream>
#include <functional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

using namespace std::string_view_literals;

template <class value_type, std::ranges::range R>
constexpr value_type construct_string(R&& r) {
  // https://stackoverflow.com/a/68121694
  return value_type(&*r.begin(), std::ranges::distance(r));
}

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

template <class return_t = std::string_view, bool keep_spaces = false>
struct trimmer_base {
  template <class R>
  constexpr return_t operator()(R&& r) const {
    auto str = construct_string<std::string_view>(std::forward<R>(r));
    if constexpr (keep_spaces) {
      return return_t{trim(str, "\t\n\r\f\v")};
    } else {
      return return_t{trim(str)};
    }
  }
};
template <class return_t = std::string_view>
using trimmer = trimmer_base<return_t, false>;
template <class return_t = std::string_view>
using trimmer_keep_spaces = trimmer_base<return_t, true>;

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

template <class T>
concept readfile_invocable_no_filenum = std::invocable<T, std::string_view>;
template <class T>
concept readfile_invocable_with_filenum =
    std::invocable<T, std::string_view, int>;
template <class T>
concept readfile_invocable =
    readfile_invocable_no_filenum<T> || readfile_invocable_with_filenum<T>;

template <class trim_op_t = trimmer<>, class OpT>
  requires(readfile_invocable<OpT>)
void readfile_op(const std::string& filename, OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 1; std::getline(file, line); ++linenum) {
    if constexpr (readfile_invocable_with_filenum<OpT>) {
      operation(trim_op_t{}(line), linenum);
    } else {
      operation(trim_op_t{}(line));
    }
  }
  file.close();
}

template <class trim_op_t = trimmer<>, class FirstLineOpT, class OpT>
  requires(readfile_invocable_no_filenum<FirstLineOpT> &&
           readfile_invocable<OpT>)
void readfile_op_header(const std::string& filename,
                        FirstLineOpT first_line_operation, OpT operation) {
  readfile_op(filename, [&](std::string_view line, int linenum) {
    if (linenum == 1) [[unlikely]] {
      first_line_operation(trim_op_t{}(line));
    } else [[likely]] {
      if constexpr (readfile_invocable_with_filenum<OpT>) {
        operation(trim_op_t{}(line), linenum);
      } else {
        operation(trim_op_t{}(line));
      }
    }
  });
}

std::vector<std::string> readfile_lines(const std::string& filename) {
  std::vector<std::string> lines;
  readfile_op<trimmer_keep_spaces<>>(filename, [&](std::string_view line) {
    lines.push_back(std::string{line});
  });
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

template <class output_t>
constexpr size_t max_container_elems() {
  if constexpr (is_array_class_v<output_t>) {
    return std::tuple_size<output_t>::value;
  } else {
    return std::string::npos;
  }
}

template <class value_type, std::ranges::range R>
constexpr value_type construct(R&& r) {
  if constexpr (contains_type<value_type, std::string_view, std::string>) {
    return construct_string<value_type>(std::forward<R>(r));
  } else if constexpr (std::is_arithmetic_v<value_type>) {
    auto view = construct_string<std::string_view>(std::forward<R>(r));
    return to_number<value_type>(view);
  } else {
    return value_type{r};
  }
}

template <class output_t, bool skip_empty = false, std::ranges::range R,
          class Pattern, class Proj = std::identity>
constexpr output_t split(R&& r, Pattern&& delimiter, Proj proj = {}) {
  using value_type = typename output_t::value_type;
  auto split_view = r | std::views::split(delimiter);
  output_t out;
  for (auto out_it = inserter_it(out); auto&& v : split_view) {
    if constexpr (skip_empty) {
      if (std::ranges::empty(v)) {
        continue;
      }
    }
    *out_it = construct<value_type>(proj(v));
    ++out_it;
  }
  return out;
}

static_assert(std::ranges::equal(
    std::array{"adsf", "qwret", "nvfkbdsj", "orthdfjgh", "dfjrleih"},
    split<std::array<std::string_view, 5>, true>(
        "adsf-+qwret-+nvfkbdsj-+orthdfjgh-+-+dfjrleih"sv, "-+"sv)));
