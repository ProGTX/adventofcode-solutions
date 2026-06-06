// https://adventofcode.com/2022/day/14

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>

enum SpecialChar : char {
  empty = '.',
  rock = '#',
  start = '+',
  sand = 'o',
};

using RockLine = std::array<point, 2>;
using RockLines = Vec<RockLine>;
using CaveMap = aoc::grid<char>;
constexpr let DEBUG_PRINT = false;

fn parse_rock_lines(str line) -> RockLines {
  let rock_pairs = aoc::split_to_vec(line, " -> ");
  let to_point = [](str s) { return aoc::split<point>(aoc::trim(s), ','); };
  return rock_pairs |
         stdv::adjacent<2> |
         stdv::transform([&](let& pair) -> RockLine {
           let & [ a, b ] = pair;
           return {to_point(a), to_point(b)};
         }) |
         aoc::ranges::to<RockLines>();
}

auto parse(String const& filename) -> RockLines {
  return aoc::views::read_lines(filename) |
         stdv::transform(parse_rock_lines) |
         stdv::join |
         aoc::ranges::to<RockLines>();
}

template <bool with_ground>
auto make_cave_map(RockLines const& input) -> std::pair<CaveMap, point> {
  auto rock_lines = input;
  auto min_max = aoc::min_max_helper{};
  for (let& rock_line : rock_lines) {
    min_max.update(rock_line[0]);
    min_max.update(rock_line[1]);
  }

  auto sand_start = point{500, 0};
  min_max.update(sand_start);

  if constexpr (with_ground) {
    let height = min_max.max_value.y - min_max.min_value.y + 2;
    let rock_line = RockLine{sand_start + point{-height, height},
                             sand_start + point{height, height}};
    min_max.update(rock_line[0]);
    min_max.update(rock_line[1]);
    rock_lines.push_back(rock_line);
  }

  let cave_dimensions = min_max.grid_size();
  auto cave_map = CaveMap(empty, cave_dimensions.y, cave_dimensions.x);

  let adjust_coordinates = [&](point const& p) {
    return p - min_max.min_value;
  };
  let modify_cave_map = [&](char value, point const& p) {
    let coords = adjust_coordinates(p);
    cave_map.modify(value, coords.y, coords.x);
  };

  for (let& rock_line : rock_lines) {
    auto diff = rock_line[1] - rock_line[0];
    auto step = point{(diff.x > 0) - (diff.x < 0), (diff.y > 0) - (diff.y < 0)};
    auto pos = rock_line[0];
    while (pos != rock_line[1]) {
      modify_cave_map(rock, pos);
      pos = pos + step;
    }
    modify_cave_map(rock, rock_line[1]);
  }

  modify_cave_map(start, sand_start);

  if constexpr (DEBUG_PRINT) {
    cave_map.print_all();
  }

  return {std::move(cave_map), adjust_coordinates(sand_start)};
}

template <bool with_ground>
auto solve_case(RockLines const& input) -> int {
  auto [cave_map, adjusted_start] = make_cave_map<with_ground>(input);

  let is_valid_index = [&](int row, int column) {
    return (row >= 0) &&
           (row < cave_map.num_rows()) &&
           (column >= 0) &&
           (column < cave_map.row_length());
  };

  let is_empty = [&](int row, int column) {
    let v = cave_map.at(row, column);
    return v == empty || v == start;
  };

  let try_insert_grain = [&](int row, int column) {
    auto within_bounds = true;
    while (within_bounds) {
      auto moved = false;
      for (auto [dy, dx] : {
               std::pair{1, 0},  // down
               std::pair{1, -1}, // left
               std::pair{1, 1},  // right
           }) {
        let y = row + dy;
        let x = column + dx;
        if (!is_valid_index(y, x)) {
          row = y;
          column = x;
          within_bounds = false;
          moved = true;
          break;
        }
        if (is_empty(y, x)) {
          row = y;
          column = x;
          moved = true;
          break;
        }
      }
      if (!moved) {
        within_bounds = false;
      }
    }
    if (is_valid_index(row, column) && is_empty(row, column)) {
      cave_map.modify(sand, row, column);
      return true;
    } else {
      return false;
    }
  };

  // Simulate sand falling
  auto num_grains = 0;
  let index_increase = cave_map.row_length();
  while (true) {
    auto sand_success = false;
    auto previous_index =
        cave_map.linear_index(adjusted_start.y, adjusted_start.x);
    // First go down from the source to the lowest level
    for (auto linear_index = previous_index + index_increase;
         (linear_index < cave_map.size()); linear_index += index_increase) {
      let value = cave_map.data()[linear_index];
      if (value == start) {
        AOC_UNREACHABLE("This condition shouldn't be reached");
      }
      if ((value == rock) || (value == sand)) {
        // Found solid ground, try to insert it above
        let row = static_cast<int>(previous_index / index_increase);
        let column = static_cast<int>(previous_index % index_increase);
        if (try_insert_grain(row, column)) {
          sand_success = true;
          break;
        } else {
          break;
        }
      }
      previous_index = linear_index;
    }
    if (!sand_success) {
      break;
    }
    ++num_grains;
  }

  if constexpr (DEBUG_PRINT) {
    cave_map.print_all();
  }

  return num_grains;
}

int main() {
  std::println("Part 1");
  let example = parse("day14.example");
  AOC_EXPECT_RESULT(24, solve_case<false>(example));
  let input = parse("day14.input");
  AOC_EXPECT_RESULT(655, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(93, solve_case<true>(example));
  AOC_EXPECT_RESULT(26484, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
