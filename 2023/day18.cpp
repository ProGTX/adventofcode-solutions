// https://adventofcode.com/2023/day/18

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using lagoon_t = aoc::grid<int>;
constexpr inline int black = 0;
constexpr inline int red = 0xFF0000;

constexpr void print(const lagoon_t& lagoon) {
  if (!std::is_constant_evaluated()) {
    lagoon.print_all([&](std::ostream& out, int row, int column) {
      auto color = lagoon.at(row, column);
      if (color == black) {
        out << '.';
      } else {
        out << '#';
      }
    });
  }
}

// Similar to num_inside from day10
// North facing trench tiles are those that have a trench tile to their north
// To simplify checks, lagoon should always start with an empty row
// https://en.wikipedia.org/wiki/Point_in_polygon
lagoon_t fill(const lagoon_t& input) {
  lagoon_t output{input};
  // Skip the first, empty row
  for (int row = 1; row < input.num_rows(); ++row) {
    bool inside = false;
    auto index = input.linear_index(row, 0);
    for (int col = 0; col < input.num_columns(); ++col, ++index) {
      auto& color = output.at_index(index);
      const bool is_trench = (color != black);
      if (is_trench) {
        const auto north_index = index - input.row_length();
        if (input.at_index(north_index) != black) {
          // North facing
          inside = !inside;
        } else {
          // Not north facing, do nothing
        }
      } else if (inside) {
        // Using red should be sufficient for part 1
        color = red;
      }
    }
  }
  return output;
}

lagoon_t get_lagoon(const std::map<point, int>& trench_map,
                    const aoc::min_max_helper& min_max) {
  const auto grid_size = min_max.grid_size();
  // Ensure the lagoon always starts with an empty row
  lagoon_t lagoon(black, grid_size.y + 1, grid_size.x);
  for (auto&& [current, color] : trench_map) {
    auto pos = current - min_max.min_value + point{0, 1};
    lagoon.modify(color, pos.y, pos.x);
  }
  return lagoon;
}

template <bool>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;
  std::map<point, int> trench_map;

  aoc::min_max_helper min_max;
  point current{};
  constexpr int color_base = 16;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    const char direction = line[0];
    auto [num_cubes_str, color_str] =
        aoc::split<std::array<std::string_view, 2>>(line.substr(2), ' ');
    const auto num_cubes = aoc::to_number<int>(num_cubes_str);
    const auto color = aoc::to_number<int>(color_str.substr(2), color_base);
    const auto diff = aoc::get_diff([direction]() {
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
          AOC_ASSERT(false, "Invalid direction");
          return aoc::southeast;
      }
    }());
    for (int i = 0; i < num_cubes; ++i) {
      min_max.update(current);
      trench_map.emplace(current, color);
      current += diff;
    }
  }

  auto lagoon = fill(get_lagoon(trench_map, min_max));

  int sum = 0;
  sum = std::ranges::count(lagoon, false, aoc::equal_to_value{black});
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(62, (solve_case<false>("day18.example")));
  AOC_EXPECT_RESULT(40745, (solve_case<false>("day18.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(51, (solve_case<true>("day18.example")));
  // AOC_EXPECT_RESULT(8026, (solve_case<true>("day18.input")));
  AOC_RETURN_CHECK_RESULT();
}
