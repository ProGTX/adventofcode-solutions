#ifndef AOC_STRING_H
#define AOC_STRING_H

#include "compiler.h"
#include "concepts.h"
#include "flat.h"
#include "range_to.h"
#include "utility.h"

#ifndef AOC_MODULE_SUPPORT
#include <array>
#include <concepts>
#include <exception>
#include <fstream>
#include <functional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

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
namespace detail {
// The simple way of using std::min triggers the UB sanitizer
// and there's no way to suppress it (even though it's safe),
// so we have a separate function to get the size of the suffix to remove
constexpr auto min_suffix_trim_removal(std::string_view str, std::size_t pos) {
  if (pos != std::string::npos) {
    return str.length() - pos - 1;
  } else {
    return str.length();
  }
}
} // namespace detail

constexpr const std::string_view all_whitespace = " \t\n\r\f\v";

constexpr std::string_view rtrim(std::string_view str,
                                 std::string_view whitespace) {
  const auto pos(str.find_last_not_of(whitespace));
  str.remove_suffix(detail::min_suffix_trim_removal(str, pos));
  return str;
}
constexpr std::string_view trim(std::string_view str,
                                std::string_view whitespace = all_whitespace) {
  str = ltrim(str, whitespace);
  str = rtrim(str, whitespace);
  return str;
}

template <class return_t = void, bool keep_spaces = false>
struct trimmer_base {
  template <class R>
  constexpr auto operator()(R&& r) const {
    using actual_ret_t = std::conditional_t<
        std::same_as<return_t, void>,
        std::conditional_t<std::same_as<std::remove_cvref_t<R>, std::string>,
                           std::string, std::string_view>,
        return_t>;
    auto str = construct_string<std::string_view>(std::forward<R>(r));
    constexpr std::string_view whitespace = [] {
      if constexpr (keep_spaces) {
        return all_whitespace.substr(1);
      } else {
        return all_whitespace;
      }
    }();
    return construct_string<actual_ret_t>(trim(str, whitespace));
  }
};
template <class return_t = void>
using trimmer = trimmer_base<return_t, false>;

struct keep_empty {};
struct keep_spaces {};

namespace detail {

template <class trimmer_t, bool keep_empty_lines>
std::string read_line(std::istream& stream) {
  std::string line;
  if constexpr (keep_empty_lines) {
    std::getline(stream, line);
  } else {
    while (std::getline(stream, line)) {
      if (!line.empty()) {
        break;
      }
    }
  }
  return trimmer_t{}(line);
}

template <class trimmer_t, bool keep_empty_lines>
struct full_line {
  constexpr operator std::string_view() const { return m_full_line; }
  constexpr operator std::string&() { return m_full_line; }
  constexpr operator const std::string&() const { return m_full_line; }

  // https://stackoverflow.com/q/74846868
  friend std::istream& operator>>(std::istream& stream, full_line& rhs) {
    rhs.m_full_line = detail::read_line<trimmer_t, keep_empty_lines>(stream);
    return stream;
  }

 private:
  std::string m_full_line;
};

template <class... Args>
constexpr auto read_lines_from_file(std::ifstream& file, Args...) {
  constexpr bool should_keep_spaces = contains_uncvref<keep_spaces, Args...>;
  using trimmer_t = trimmer_base<std::string, should_keep_spaces>;

  constexpr bool keep_empty_lines = contains_uncvref<keep_empty, Args...>;

  return std::views::istream<full_line<trimmer_t, keep_empty_lines>>(file);
}

} // namespace detail

// https://stackoverflow.com/a/2602258
std::string read_file(const std::string& filename) {
  auto file = std::ifstream{filename};
  auto buffer = std::stringstream{};
  buffer << file.rdbuf();
  return buffer.str();
}

namespace views {

struct __read_lines {
 public:
  template <class... Args>
  constexpr auto operator()(const std::string& filename, Args... args) {
    m_file = std::ifstream{filename};
    return detail::read_lines_from_file(m_file, std::forward<Args>(args)...);
  }
  template <class... Args>
  constexpr auto operator()(std::ifstream& file, Args... args) const {
    return detail::read_lines_from_file(file, std::forward<Args>(args)...);
  }

 private:
  std::ifstream m_file;
};

inline __read_lines read_lines{};

template <class T, class... Args>
constexpr auto read_numbers(const std::string& filename, Args... args) {
  return read_lines(filename, std::forward<Args>(args)...) |
         views::to_number<T>();
}
template <class T, class... Args>
constexpr auto read_numbers(std::ifstream& file, Args... args) {
  return read_lines(file, std::forward<Args>(args)...) | views::to_number<T>();
}

struct match_indices_view : std::ranges::view_interface<match_indices_view> {
 private:
  std::string_view haystack;
  std::string_view needle;

 public:
  constexpr match_indices_view(std::string_view haystack,
                               std::string_view needle) noexcept
      : haystack{haystack}, needle{needle} {}

  struct iterator {
   private:
    std::string_view haystack;
    std::string_view needle;
    std::size_t pos;
    std::size_t next;

   public:
    constexpr iterator(std::string_view haystack, std::string_view needle,
                       std::size_t start)
        : haystack{haystack}, needle{needle}, pos{start} {
      advance();
    }
    constexpr void advance() { next = haystack.find(needle, pos); }
    constexpr auto operator*() const {
      return std::pair{next, haystack.substr(next, needle.size())};
    }
    constexpr iterator& operator++() {
      pos = next + needle.size();
      advance();
      return *this;
    }
    constexpr bool operator==(std::default_sentinel_t) const {
      return next == std::string_view::npos;
    }
  };

  constexpr iterator begin() const { return iterator{haystack, needle, 0}; }
  constexpr std::default_sentinel_t end() const { return {}; }
};

namespace detail {

struct match_indices_fn {
  std::string_view needle;
  constexpr auto operator()(std::string_view haystack) const {
    return match_indices_view{haystack, needle};
  }
  constexpr friend auto operator|(std::string_view haystack,
                                  const match_indices_fn& fn) {
    return fn(haystack);
  }
};

} // namespace detail

constexpr auto match_indices(std::string_view needle) {
  return detail::match_indices_fn{needle};
}

} // namespace views

constexpr std::size_t pattern_size(std::string_view str) { return str.size(); }
constexpr std::size_t pattern_size(const std::string& str) {
  return str.size();
}
constexpr std::size_t pattern_size(char) { return 1; }
constexpr std::size_t pattern_size(const char* str) {
  return pattern_size(std::string_view{str});
}
template <std::size_t N>
constexpr std::size_t pattern_size(const char[N]) {
  return N;
}

template <class... Args>
constexpr std::string read_line(std::ifstream& stream, Args... args) {
  auto view = views::read_lines(stream, std::forward<Args>(args)...);
  return *view.begin();
}

template <class value_type, std::ranges::range R>
constexpr value_type construct(R&& r) {
  if constexpr (contains_uncvref<value_type, std::string_view, std::string>) {
    return construct_string<value_type>(std::forward<R>(r));
  } else if constexpr (std::is_arithmetic_v<value_type>) {
    auto view = construct_string<std::string_view>(std::forward<R>(r));
    return to_number<value_type>(view);
  } else {
    return value_type{std::forward<R>(r)};
  }
}

/**
 * Splits a char range r based on the delimiter.
 *
 * The output type controls a lot of the behavior of the function.
 * It must be a container-like type that can be iterated over
 * and has value_type defined.
 * If not specified, defaults to vector<string_view>.
 *
 * skip_empty and the projection control what happens to the intermediate
 * elements. If skip_empty is true, empty element won't be stored.
 * The projection is applied to the element before storing.
 *
 * @code
 * auto result = aoc::split("hello world", ' ');
 * static_assert(std::same_as<decltype(result), std::vector<std::string_view>>);
 * assert(result[0] == "hello");
 * assert(result[1] == "world");
 * @endcode
 *
 * @code
 * auto result = aoc::split<std::vector<std::string>>("hello world", ' ');
 * static_assert(std::same_as<decltype(result), std::vector<std::string>>);
 * assert(result[0] == "hello");
 * assert(result[1] == "world");
 * @endcode
 *
 * @code
 * // A container of numbers instead of string/string_view
 * // actually parses the intermediate string_view.
 * auto result = aoc::split<std::array<int, 3>>("1 2 3", ' ');
 * static_assert(std::same_as<decltype(result), std::array<int, 3>);
 * assert(result[0] == 1);
 * assert(result[1] == 2);
 * assert(result[2] == 3);
 * @endcode
 *
 * @code
 * // Note that when the output is an array, the last element doesn't contain
 * // the rest of the string, it's just discarded.
 * auto result = aoc::split<std::array<std::string_view, 2>>(
 *                  "hello world today", ' ');
 * static_assert(std::same_as<
 *                 decltype(result),
 *                 std::array<std::string_view, 2>>);
 * assert(result[0] == "hello");
 * assert(result[1] == "world");
 * // "today" is not included in result
 * @endcode
 */
template <class output_t = void, bool skip_empty = false, std::ranges::range R,
          class Pattern, class Proj = std::identity>
constexpr auto split(R&& r, Pattern&& delimiter, Proj proj = {}) {
  auto out = [] {
    if constexpr (std::same_as<output_t, void>) {
      return std::vector<std::string_view>{};
    } else {
      return output_t{};
    }
  }();
  using value_type = typename decltype(out)::value_type;
  auto split_view = [&]() {
    if constexpr (std::same_as<std::decay_t<Pattern>, const char*>) {
      return r | std::views::split(std::string_view{delimiter});
    } else {
      return r | std::views::split(delimiter);
    }
  }();
  constexpr const auto fixed_capacity = max_container_elems<decltype(out)>();

  int count = 0;
  for (auto out_it = inserter_it(out); auto&& v : split_view) {
    if constexpr (skip_empty) {
      if (std::ranges::empty(v)) {
        continue;
      }
    }
    if constexpr (fixed_capacity != std::string::npos) {
      if (count >= fixed_capacity) {
        break;
      }
    }
    *out_it = construct<value_type>(proj(v));
    ++out_it;
    if constexpr (fixed_capacity != std::string::npos) {
      ++count;
    }
  }
  return out;
}
static_assert(std::ranges::equal(
    std::array{"adsf", "qwret", "nvfkbdsj", "orthdfjgh", "dfjrleih"},
    split<std::array<std::string_view, 5>, true>(
        "adsf-+qwret-+nvfkbdsj-+orthdfjgh-+-+dfjrleih"sv, "-+"sv)));

/**
 * Simplified version of split that stores output to std::array<value_type, N>.
 *
 * If the string splits into more than N elements,
 * everything after the Nth element is discarded.
 */
template <std::size_t N, class value_type = std::string_view,
          bool skip_empty = false, std::ranges::range R, class Pattern,
          class Proj = std::identity>
constexpr auto split_to_array(R&& r, Pattern&& delimiter, Proj proj = {}) {
  return split<std::array<value_type, N>, skip_empty>(
      std::forward<R>(r), std::forward<Pattern>(delimiter), std::move(proj));
}
static_assert("world" == split_to_array<2>("hello world today", " ")[1]);

/// Simplified version of split that stores output to std::vector<value_type>
template <class value_type = std::string_view, bool skip_empty = false,
          std::ranges::range R, class Pattern, class Proj = std::identity>
constexpr auto split_to_vec(R&& r, Pattern&& delimiter, Proj proj = {}) {
  return split<std::vector<value_type>, skip_empty>(
      std::forward<R>(r), std::forward<Pattern>(delimiter), std::move(proj));
}

/**
 * Splits a string view into exactly two parts.
 *
 * The first part contains everything before the delimiter,
 * and the second part everything after.
 * If the delimiter is not found, then the second part is an empty string.
 *
 * Works similar to split<std::array<value_type, 2>>,
 * but in case of more than 2 elements doesn't split the rest of the string.
 *
 * @code
 * auto result = aoc::split_once("hello world today", ' ');
 * static_assert(std::same_as<
 *                 decltype(result),
 *                 std::array<std::string_view, 2>>);
 * assert(result[0] == "hello");
 * assert(result[1] == "world today");
 * @endcode
 *
 * @code
 * auto result = aoc::split_once<int>("1 2", ' ');
 * static_assert(std::same_as<
 *                 decltype(result),
 *                 std::array<int, 2>>);
 * assert(result[0] == 1);
 * assert(result[1] == 2);
 * // Note that if we passed "1 2 3", the second part would be "2 3",
 * // which would fail to parse as an int.
 * @endcode
 */
template <class value_type = std::string_view, class Pattern,
          class Proj = std::identity>
constexpr auto split_once(std::string_view str, Pattern&& delimiter,
                          Proj proj = {}) {
  const auto pos = str.find(delimiter);
  return std::array<value_type, 2>{
      construct<value_type>(proj(str.substr(0, pos))),
      construct<value_type>(
          proj((pos == std::string::npos)
                   ? ""
                   : str.substr(pos + pattern_size(delimiter)))),
  };
}
static_assert("world today" == split_once("hello world today", ' ')[1]);
static_assert(2 == aoc::split_once<int>("1 2", ' ')[1]);

/**
 * split_sstream takes a slightly different approach to splitting a string
 * than `split`, by not specifying a delimiter, but the delimiter is determined
 * based on the `value_type`, exactly like how std::istringstream works.
 *
 * If value_type is std::string, the string will be split based on
 * any whitespace. If it's a number type, the function will collect
 * all numbers it can extract from the string.
 *
 * @code
 * auto result = split_sstream("hello    world today  ");
 * static_assert(std::same_as<
 *                 decltype(result),
 *                 std::vector<std::string>>);
 * assert(result.size() == 3);
 * assert(result[0] == "hello");
 * assert(result[1] == "world");
 * assert(result[2] == "today");
 * @endcode
 *
 * @code
 * auto result = split_sstream<int>("3453m6546 254, 6323,2342ad84");
 * static_assert(std::same_as<
 *                 decltype(result),
 *                 std::vector<int>>);
 * assert(result.size() == 6);
 * assert(result[0] == 3453);
 * assert(result[1] == 6546);
 * assert(result[2] == 254);
 * assert(result[3] == 6323);
 * assert(result[4] == 2342);
 * assert(result[5] == 84);
 * @endcode
 *
 * IMPORTANT: You can't split into string_views before C++26.
 *
 * Unfortunately istringstream isn't constexpr even in C++26.
 *
 * TODO: Support different output containers.
 */
template <class value_type = std::string>
constexpr std::vector<value_type> split_sstream(std::string s) {
  auto stream = std::istringstream{s};
  return std::views::istream<value_type>(stream) |
         aoc::ranges::to<std::vector<value_type>>();
}
template <class value_type = std::string>
constexpr auto split_sstream(std::string_view s) {
  return split_sstream<value_type>(std::string{s});
}
template <class value_type = std::string>
constexpr auto split_sstream(const char* s) {
  return split_sstream<value_type>(std::string{s});
}

template <char one = '1', class return_t = unsigned>
constexpr return_t binary_to_number(const char* str) {
  return binary_to_number<one, return_t, std::string_view, true>(
      std::string_view{str});
}
template <char one = '1', class return_t = unsigned, size_t size>
constexpr return_t binary_to_number(const char str[size]) {
  return binary_to_number<one, return_t, std::string_view, true>(
      std::string_view{str, size});
}
template <char one = '1', class return_t = unsigned>
constexpr return_t binary_to_number(std::string_view str) {
  return binary_to_number<one, return_t, std::string_view, true>(str);
}
template <char one = '1', class return_t = unsigned>
constexpr return_t binary_to_number(const std::string& str) {
  return binary_to_number<one, return_t, std::string_view, true>(
      std::string_view{str});
}

static_assert(0 == binary_to_number<'1'>("0"));
static_assert(1 == binary_to_number<'1'>("1"));
static_assert(2 == binary_to_number("01"));
static_assert(13 == binary_to_number("1011"));
static_assert(205 == binary_to_number<'#'>("#.##..##."));
static_assert(6757 == binary_to_number<'#'>("#.#..##..#.##"));

constexpr int count_substrings(std::string_view haystack,
                               std::string_view needle) {
  const int count =
      std::ranges::distance(haystack | std::views::split(needle)) - 1;
  // In some cases the distance returned is 0
  return std::ranges::max(0, count);
}
constexpr int count_substrings(const std::string& haystack,
                               std::string_view needle) {
  return count_substrings(std::string_view{haystack}, needle);
}
constexpr int count_substrings(std::ranges::input_range auto&& haystack,
                               std::string_view needle) {
  // Much faster if we convert the haystack to a string first
  auto haystack_str = haystack | ranges::to<std::string>();
  return count_substrings(std::string_view{haystack_str}, needle);
}

static_assert(1 == count_substrings("abc", "abc"));
static_assert(2 == count_substrings("abcabc", "abc"));
static_assert(2 == count_substrings("abcyabc", "abc"));
static_assert(2 == count_substrings("abcyacabc", "abc"));
static_assert(2 == count_substrings("cbacba" | std::views::reverse, "abc"));

class name_to_id {
 public:
  constexpr name_to_id() = default;

  constexpr explicit name_to_id(std::size_t start) : current_id(start) {}

  constexpr std::size_t intern(std::string_view name) {
    const auto it = name_ids.find(name);
    if (it != std::end(name_ids)) {
      return it->second;
    }
    name_ids.emplace(std::string{name}, current_id);
    return current_id++;
  }

  constexpr std::optional<std::size_t> get(std::string_view name) const {
    const auto it = name_ids.find(name);
    if (it != std::end(name_ids)) {
      return it->second;
    }
    return {};
  }

  constexpr std::size_t expect(std::string_view name) const {
    const auto it = name_ids.find(name);
    if (it != std::end(name_ids)) {
      return it->second;
    }
    throw std::runtime_error(std::format("name '{}' not found", name));
  }

  constexpr std::size_t new_size(std::size_t old_size) const {
    return std::ranges::max(current_id, old_size);
  }

 private:
  std::size_t current_id{0};
  flat_map<std::string, std::size_t> name_ids{};
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_STRING_H
