// https://adventofcode.com/2023/day/18

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <span>

fn parse(String const& filename) -> Vec<String> {
  return aoc::read_lines(filename);
}

using point_t = aoc::point_type<i64>;
using trench_t = Vec<point_t>;

fn trench_step(aoc::facing_t direction, i64 num_cubes)
    -> std::pair<point_t, i64> {
  return {aoc::get_diff<i64>(direction), num_cubes};
}

fn lagoon_size(trench_t const& trench, i64 trench_length) -> i64 {
  let area = aoc::calculate_area<i64>(std::span{trench});
  // https://en.wikipedia.org/wiki/Pick%27s_theorem
  // A = i + b/2 - 1
  //   -> i = A - b/2 + 1
  // But we also need to add back b to account for the area of the edge
  //   -> A + b/2 + 1
  return area + (trench_length / 2) + 1;
}

template <class ParseLine>
fn solve(Vec<String> const& lines, ParseLine parse_line) -> i64 {
  auto trench = trench_t{};
  auto trench_length = i64{0};
  auto current = point_t{};

  for (str line : lines) {
    let[diff, num_cubes] = parse_line(line);
    trench.push_back(current);
    current += diff * num_cubes;
    trench_length += num_cubes;
  }

  return lagoon_size(trench, trench_length);
}

fn solve_case1(Vec<String> const& lines) -> i64 {
  return solve(lines, [](str line) {
    let direction = line[0];
    let[num_cubes_str, color_str] = aoc::split_once(line.substr(2), ' ');
    let num_cubes = aoc::to_number<i64>(num_cubes_str);
    return trench_step(
        [direction]() {
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
        }(),
        num_cubes);
  });
}

fn solve_case2(Vec<String> const& lines) -> i64 {
  static constexpr let hex_base = 16;
  static constexpr let hex_direction_mapping = std::array{
      aoc::east,
      aoc::south,
      aoc::west,
      aoc::north,
  };

  return solve(lines, [](str line) {
    let[num_cubes_str, hex_str] = aoc::split_once(line.substr(2), ' ');
    let num_cubes = aoc::to_number<i64>(hex_str.substr(2, 5), hex_base);
    return trench_step(hex_direction_mapping[aoc::to_number(hex_str[7])],
                       num_cubes);
  });
}

int main() {
  std::println("Part 1");
  let example = parse("day18.example");
  AOC_EXPECT_RESULT(62, solve_case1(example));
  let input = parse("day18.input");
  AOC_EXPECT_RESULT(40745, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(952408144115, solve_case2(example));
  AOC_EXPECT_RESULT(90111113594927, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
