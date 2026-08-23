// https://adventofcode.com/2016/day/2

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <array>
#include <print>
#endif

using Input = Vec<String>;

auto parse(String const& filename) -> Input {
  return aoc::read_lines(filename);
}

fn to_diff(char direction) -> point {
  switch (direction) {
    case 'U':
      return aoc::get_diff<int>(aoc::north);
    case 'D':
      return aoc::get_diff<int>(aoc::south);
    case 'L':
      return aoc::get_diff<int>(aoc::west);
    case 'R':
      return aoc::get_diff<int>(aoc::east);
    default:
      AOC_UNREACHABLE("Invalid direction");
  }
}

template <bool Complicated>
fn solve_case(Input const& input) -> String {
  constexpr let keypad = [] {
    if constexpr (!Complicated) {
      return aoc::fixed_char_grid<3, 3>("123"
                                        "456"
                                        "789");
    } else {
      return aoc::fixed_char_grid<5, 5>("  1  "
                                        " 234 "
                                        "56789"
                                        " ABC "
                                        "  D  ");
    }
  }();

  auto pos = aoc::as_consteval(
      keypad.position(aoc::ranges::position(keypad, '5').value()));
  auto code = String{};

  for (let& line : input) {
    for (let direction : line) {
      let new_pos = pos + to_diff(direction);
      if (keypad.in_bounds(new_pos.y, new_pos.x) &&
          (!Complicated || (keypad.at(new_pos.y, new_pos.x) != ' '))) {
        pos = new_pos;
      }
    }
    code.push_back(keypad.at(pos.y, pos.x));
  }

  return code;
}

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT("1985", solve_case<false>(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT("73597", solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("5DB3", solve_case<true>(example));
  AOC_EXPECT_RESULT("A47DA", solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
