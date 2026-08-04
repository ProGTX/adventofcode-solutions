// https://adventofcode.com/2015/day/12

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <cctype>
#include <print>
#include <ranges>
#include <variant>
#endif

fn sum(str s_view) -> i32 {
  auto value = 0;
  while (!s_view.empty()) {
    // Positions instead of iterators:
    // searching a `substr` yields iterators into that temporary view,
    // so comparing them against iterators of `s_view` mixes two ranges
    constexpr let digits = str{"0123456789"};
    let digit_pos = s_view.find_first_of(digits);
    if (digit_pos == str::npos) {
      break;
    }
    AOC_ASSERT(digit_pos > 0, "Assuming number not at beginning of string");
    let nondigit_pos = s_view.find_first_not_of(digits, digit_pos + 1);
    AOC_ASSERT(nondigit_pos != str::npos,
               "Assuming number is never at the end");
    let current =
        aoc::to_number<i32>(s_view.substr(digit_pos, nondigit_pos - digit_pos));
    value += (s_view[digit_pos - 1] == '-') ? -current : current;
    s_view = s_view.substr(nondigit_pos);
  }
  return value;
}

namespace ScopeType {
struct Object {
  bool red;
};
struct Array {};
using Any = std::variant<Object, Array>;
} // namespace ScopeType

fn sum_non_red(str s) -> i32 {
  auto scope_stack = Vec<ScopeType::Any>{};
  auto value_stack = Vec<i32>{};
  value_stack.push_back(0);
  auto value_str = String{};
  for (let c : s) {
    if ((c == '-') || std::isalnum(c)) {
      value_str.push_back(c);
    } else if (!value_str.empty()) {
      let first = value_str.front();
      if ((first == '-') || std::isdigit(first)) {
        let value = aoc::to_number<i32>(value_str);
        auto& top = value_stack.back();
        top += value;
      } else if (value_str == "red") {
        scope_stack.back() = ScopeType::Object{true};
      }
      value_str.clear();
    }

    let red = [&] {
      if (scope_stack.empty()) {
        return false;
      }
      let* top_object = std::get_if<ScopeType::Object>(&scope_stack.back());
      return top_object && top_object->red;
    }();
    switch (c) {
      case '{': {
        value_stack.push_back(0);
        scope_stack.push_back(ScopeType::Object{false});
        break;
      };
      case '}': {
        let top = value_stack.back();
        value_stack.pop_back();
        if (!red) {
          value_stack.back() += top;
        }
        scope_stack.pop_back();
        break;
      };
      case '[': {
        value_stack.push_back(0);
        scope_stack.push_back(ScopeType::Array{});
        break;
      };
      case ']': {
        let top = value_stack.back();
        value_stack.pop_back();
        value_stack.back() += top;
        scope_stack.pop_back();
        break;
      };
      default:
        break;
    }
  }
  return value_stack.back();
}

String parse(String const& filename) { return aoc::read_single_line(filename); }

template <bool red>
fn solve_case(str line) -> i32 {
  if constexpr (!red) {
    return sum(line);
  } else {
    return sum_non_red(line);
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day12.example");
  AOC_EXPECT_RESULT(6, solve_case<false>(example));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(156366, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4, solve_case<true>(example));
  AOC_EXPECT_RESULT(96852, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
