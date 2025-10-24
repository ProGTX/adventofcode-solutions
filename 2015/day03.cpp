// https://adventofcode.com/2015/day/3

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <print>
#include <string>
#include <string_view>

template <int num_santas>
int deliveries(std::string_view directions) {
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

int solve_case1(const std::string& filename) {
  std::ifstream file{filename};
  return deliveries<1>(aoc::read_line(file));
}

int solve_case2(const std::string& filename) {
  std::ifstream file{filename};
  return deliveries<2>(aoc::read_line(file));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(4, solve_case1("day03.example"));
  AOC_EXPECT_RESULT(2565, solve_case1("day03.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(3, solve_case2("day03.example"));
  AOC_EXPECT_RESULT(2639, solve_case2("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
