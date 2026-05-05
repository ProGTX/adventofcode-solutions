// https://adventofcode.com/2022/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>
#include <stdexcept>

using Moves = Vec<point>;
using Visited = aoc::flat_set<point>;

fn parse_move(str line) -> point {
  let[operation, value_str] = aoc::split_once(line, ' ');
  let value = aoc::to_number<int>(value_str);
  switch (operation[0]) {
    case 'R':
      return {value, 0};
    case 'L':
      return {-value, 0};
    case 'U':
      return {0, value};
    case 'D':
      return {0, -value};
    default:
      throw std::runtime_error("Invalid command " + String{operation});
  }
}

auto parse(String const& filename) -> Moves {
  return aoc::views::read_lines(filename) |
         std::views::transform(parse_move) |
         aoc::ranges::to<Moves>();
}

template <usize num_knots>
fn move_tail(std::array<point, num_knots>& rope, Visited& visited) {
  for (usize i = 1; i < num_knots; ++i) {
    let in_front = rope[i - 1];
    auto& current = rope[i];
    let diff = in_front - current;
    let diff_abs = diff.abs();
    if (diff_abs.x >= 2 || diff_abs.y >= 2) {
      current += diff.normal();
    }
  }
  visited.insert(rope.back());
}

template <usize num_knots>
fn solve_case(Moves const& moves) -> usize {
  auto rope = std::array<point, num_knots>{};
  auto& head = rope.front();

  auto visited = Visited{};
  visited.insert(rope.back());

  for (let diff : moves) {
    // Can only move vertically or horizontally
    if ((diff.x * diff.y) != 0) {
      throw std::runtime_error("Diagonal movement");
    }
    let diff_abs = diff.abs();
    let delta = diff.normal();
    if (diff.x != 0) {
      for (int i = 0; i < diff_abs.x; ++i) {
        head.x += delta.x;
        move_tail(rope, visited);
      }
    } else {
      for (int i = 0; i < diff_abs.y; ++i) {
        head.y += delta.y;
        move_tail(rope, visited);
      }
    }
  }

  return visited.size();
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(13, solve_case<2>(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(5695, solve_case<2>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1, solve_case<10>(example));
  let example2 = parse("day09.example2");
  AOC_EXPECT_RESULT(36, solve_case<10>(example2));
  AOC_EXPECT_RESULT(2434, solve_case<10>(input));

  AOC_RETURN_CHECK_RESULT();
}
