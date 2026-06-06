// https://adventofcode.com/2022/day/24

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>

using Arrow = aoc::arrow_type<int>;
using Blizzards = std::vector<Arrow>;
using Input = std::pair<point, Blizzards>;

auto parse(String const& filename) -> Input {
  let grid = aoc::read_char_grid(filename);
  let num_rows = static_cast<int>(grid.num_rows());
  let num_columns = static_cast<int>(grid.num_columns());
  auto blizzards = std::vector<Arrow>{};
  for (int row = 1; row < num_rows - 1; ++row) {
    for (int col = 1; col < num_columns - 1; ++col) {
      let c = grid.at(row, col);
      if (c == '>' || c == '<' || c == '^' || c == 'v') {
        blizzards.push_back(
            {.position = {col, row}, .direction = aoc::to_facing(c)});
      }
    }
  }
  return {{num_columns, num_rows}, std::move(blizzards)};
}

fn solve_case1(Input const& input) -> u32 { return 0; }

int main() {
  std::println("Part 1");
  let example = parse("day24.example");
  AOC_EXPECT_RESULT(10, solve_case1(example));
  let example2 = parse("day24.example2");
  AOC_EXPECT_RESULT(18, solve_case1(example2));
  let input = parse("day24.input");
  AOC_EXPECT_RESULT(1337, solve_case1(input));

  std::println("Part 2");
  // AOC_EXPECT_RESULT(1337, solve_case2(example));
  // AOC_EXPECT_RESULT(1337, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
