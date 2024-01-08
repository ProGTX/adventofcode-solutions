// https://adventofcode.com/2022/day/18

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using droplet_t = aoc::cube_type<int>;
using droplets_t = std::set<droplet_t>;

int calc_surface_area(const droplets_t& droplets) {
  int area = 0;
  const auto add_side_area = [&](const droplet_t& droplet,
                                 const droplet_t& move) {
    if (droplets.find(droplet + move) != std::end(droplets)) {
      return;
    }
    ++area;
  };
  for (const auto& droplet : droplets) {
    add_side_area(droplet, {0, 0, 1});
    add_side_area(droplet, {0, 1, 0});
    add_side_area(droplet, {1, 0, 0});
    add_side_area(droplet, {0, 0, -1});
    add_side_area(droplet, {0, -1, 0});
    add_side_area(droplet, {-1, 0, 0});
  }
  return area;
}

template <bool>
int solve_case(const std::string& filename) {
  droplets_t droplets;

  aoc::readfile_op(filename, [&](std::string_view line) {
    auto [x, y, z] = aoc::split<std::array<int, 3>>(line, ',');
    droplets.emplace(x, y, z);
  });

  auto area = calc_surface_area(droplets);
  std::cout << filename << " -> " << area << std::endl;
  return area;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(10, solve_case<false>("day18.example"));
  AOC_EXPECT_RESULT(64, solve_case<false>("day18.example2"));
  AOC_EXPECT_RESULT(4310, solve_case<false>("day18.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(10, solve_case<true>("day18.example"));
  AOC_EXPECT_RESULT(58, solve_case<true>("day18.example2"));
  AOC_RETURN_CHECK_RESULT();
}
