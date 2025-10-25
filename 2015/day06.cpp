// https://adventofcode.com/2015/day/6

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

using namespace std::string_view_literals;
// Include spaces in strings so we don't have to trim later
static constexpr const auto turn_on = "turn on "sv;
static constexpr const auto turn_off = "turn off "sv;
static constexpr const auto toggle = "toggle "sv;
static constexpr const auto through = " through "sv;

using cell_t = unsigned;

namespace part1 {
constexpr cell_t turn_on_fn(cell_t) { return 1; }
constexpr cell_t turn_off_fn(cell_t) { return 0; }
constexpr cell_t toggle_fn(cell_t c) { return (c == 1) ? 0 : 1; }
} // namespace part1

namespace part2 {
constexpr cell_t turn_on_fn(cell_t c) { return c + 1; }
constexpr cell_t turn_off_fn(cell_t c) { return (c == 0) ? 0 : (c - 1); }
constexpr cell_t toggle_fn(cell_t c) { return c + 2; }
} // namespace part2

using op_fn_t = cell_t (*)(cell_t);

template <bool brightness>
unsigned solve_case(const std::string& filename) {
  using boundaries_t = std::array<std::string_view, 2>;
  auto lights = aoc::grid<cell_t>{0, 1000, 1000};
  for (std::string_view line : aoc::views::read_lines(filename)) {
    op_fn_t operation = nullptr;
    boundaries_t boundaries;
    if (line.starts_with(turn_on)) {
      if constexpr (!brightness) {
        operation = &part1::turn_on_fn;
      } else {
        operation = &part2::turn_on_fn;
      }
      boundaries =
          aoc::split<boundaries_t>(line.substr(turn_on.size()), through);
    } else if (line.starts_with(turn_off)) {
      if constexpr (!brightness) {
        operation = &part1::turn_off_fn;
      } else {
        operation = &part2::turn_off_fn;
      }
      boundaries =
          aoc::split<boundaries_t>(line.substr(turn_off.size()), through);
    } else if (line.starts_with(toggle)) {
      if constexpr (!brightness) {
        operation = &part1::toggle_fn;
      } else {
        operation = &part2::toggle_fn;
      }
      boundaries =
          aoc::split<boundaries_t>(line.substr(toggle.size()), through);
    } else {
      AOC_ASSERT(false, "Invalid beginning of line")
    }
    const auto begin = aoc::split<point>(boundaries[0], ',');
    const auto end = aoc::split<point>(boundaries[1], ',');
    for (int row = begin.y; row <= end.y; ++row) {
      for (cell_t& c : lights.row_view(row) | std::views::drop(begin.x) |
                           std::views::take(end.x - begin.x + 1)) {
        c = operation(c);
      }
    }
  }
  if constexpr (!brightness) {
    return std::ranges::count(lights, static_cast<cell_t>(1));
  } else {
    return aoc::ranges::accumulate(lights, 0u);
  }
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(998996, solve_case<false>("day06.example"));
  AOC_EXPECT_RESULT(569999, solve_case<false>("day06.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(1001996, solve_case<true>("day06.example"));
  AOC_EXPECT_RESULT(17836115, solve_case<true>("day06.input"));
  AOC_RETURN_CHECK_RESULT();
}
