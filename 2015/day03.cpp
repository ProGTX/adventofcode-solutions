// https://adventofcode.com/2015/day/3

#include "../common/common.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <fstream>
#include <map>
#include <print>
#include <string>
#include <string_view>
#endif

std::string parse(const std::string& filename) {
  return aoc::read_single_line(filename);
}

template <int num_santas>
int solve_case(std::string_view directions) {
  std::array<point, num_santas> santas{};
  std::array<std::map<point, int>, num_santas> visited{};
  for (auto& houses : visited) {
    houses[point{}] = 1;
  }
  int santa_id = 0;
  for (char c : directions) {
    const auto diff = aoc::get_diff(aoc::to_facing(c));
    santas[santa_id] += diff;
    ++visited[santa_id][santas[santa_id]];
    santa_id = (santa_id + 1) % num_santas;
  }
  if constexpr (num_santas > 1) {
    static_assert(num_santas <= 2);
    visited[0].merge(std::move(visited[1]));
  }
  return visited[0].size();
}

int main() {
  std::println("Part 1");
  const auto example = parse("day03.example");
  AOC_EXPECT_RESULT(4, solve_case<1>(example));
  const auto input = parse("day03.input");
  AOC_EXPECT_RESULT(2565, solve_case<1>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3, solve_case<2>(example));
  AOC_EXPECT_RESULT(2639, solve_case<2>(input));

  AOC_RETURN_CHECK_RESULT();
}
