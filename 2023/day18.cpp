// https://adventofcode.com/2023/day/18

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;
using point_t = aoc::point_type<int_t>;
using trench_t = std::vector<point_t>;

int_t lagoon_size(const trench_t& trench, const int_t trench_length) {
  auto area = aoc::calculate_area<int_t>(std::span{trench});
  // https://en.wikipedia.org/wiki/Pick%27s_theorem
  // A = i + b/2 - 1
  //   -> i = A - b/2 + 1
  // But we also need to add back b to account for the area of the edge
  //   -> A + b/2 + 1
  return area + (trench_length / 2) + 1;
}

template <bool invert>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  const auto parse_input_p1 =
      [&](std::string_view line) -> std::pair<point_t, int_t> {
    const char direction = line[0];
    auto [num_cubes_str, color_str] = aoc::split_once(line.substr(2), ' ');
    const auto num_cubes = aoc::to_number<int_t>(num_cubes_str);
    const auto diff = aoc::get_diff<int_t>([direction]() {
      switch (direction) {
        case 'R':
          return aoc::east;
        case 'D':
          return aoc::south;
        case 'L':
          return aoc::west;
        case 'U':
          return aoc::north;
        default:
          AOC_UNREACHABLE("Invalid direction");
          return aoc::southeast;
      }
    }());
    return {diff, num_cubes};
  };

  constexpr int hex_base = 16;
  std::array hex_direction_mapping{
      aoc::east,
      aoc::south,
      aoc::west,
      aoc::north,
  };
  const auto parse_input_p2 =
      [&](std::string_view line) -> std::pair<point_t, int_t> {
    auto [num_cubes_str, hex_str] = aoc::split_once(line.substr(2), ' ');
    const auto diff =
        aoc::get_diff(hex_direction_mapping[aoc::to_number(hex_str[7])]);
    auto num_cubes = aoc::to_number<int_t>(hex_str.substr(2, 5), hex_base);
    return {diff, num_cubes};
  };

  trench_t trench;
  int_t trench_length = 0;
  point_t current{};

  for (std::string_view line : aoc::views::read_lines(filename)) {
    point_t diff;
    int num_cubes = 0;
    if constexpr (!invert) {
      std::tie(diff, num_cubes) = parse_input_p1(line);
    } else {
      std::tie(diff, num_cubes) = parse_input_p2(line);
    }
    trench.push_back(current);
    current += diff * num_cubes;
    trench_length += num_cubes;
  }

  auto sum = lagoon_size(trench, trench_length);
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(62, (solve_case<false>("day18.example")));
  AOC_EXPECT_RESULT(40745, (solve_case<false>("day18.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(952408144115, (solve_case<true>("day18.example")));
  AOC_EXPECT_RESULT(90111113594927, (solve_case<true>("day18.input")));
  AOC_RETURN_CHECK_RESULT();
}
