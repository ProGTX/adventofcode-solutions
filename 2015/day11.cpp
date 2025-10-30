// https://adventofcode.com/2015/day/11

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
using u32 = std::uint32_t;
template <class T>
using Vec = std::vector<T>;

template <bool>
std::string solve_case(const std::string& filename) {
  std::ifstream file{filename};
  auto input = aoc::read_line(file) |
               aoc::views::to_number<u32>() |
               aoc::ranges::to<std::vector<u32>>();
  return "";
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT("ghjaabcc", solve_case<false>("day11.example"));
  AOC_EXPECT_RESULT("!hxbxwxba", solve_case<false>("day11.input"));
  // std::println("Part 2");
  // AOC_EXPECT_RESULT(3369156, solve_case<50>("day11.example"));
  // AOC_EXPECT_RESULT(5103798, solve_case<50>("day11.input"));
  AOC_RETURN_CHECK_RESULT();
}
