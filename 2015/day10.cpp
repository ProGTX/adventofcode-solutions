// https://adventofcode.com/2015/day/10

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

// Carcinization
#define let const auto
using str = std::string_view;

template <int iterations>
int solve_case(const std::string& filename) {
  return 0;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(237746, solve_case<40>("day10.example"));
  AOC_EXPECT_RESULT(360154, solve_case<40>("day10.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(3369156, solve_case<50>("day10.example"));
  AOC_EXPECT_RESULT(5103798, solve_case<50>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
