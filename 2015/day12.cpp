// https://adventofcode.com/2015/day/12

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

// Carcinization
#define let const auto
#define fn constexpr auto
using u32 = std::uint32_t;
template <class T>
using Vec = std::vector<T>;
template <class T>
using Option = std::optional<T>;
using String = std::string;
using str = std::string_view;
using usize = std::size_t;

int solve_case(const String& filename) {
  std::ifstream file{filename};
  aoc::read_line(file);
  return -1;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  AOC_EXPECT_RESULT(156366, solve_case("day12.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3, solve_case("day12.example"));
  // AOC_EXPECT_RESULT(0, solve_case("day12.input"));
  AOC_RETURN_CHECK_RESULT();
}
