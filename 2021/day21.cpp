// https://adventofcode.com/2021/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <print>
#include <string>
#include <vector>

using Input = std::pair<u32, u32>;

fn parse(String const& filename) -> Input {
  auto lines = aoc::views::read_lines(filename);
  auto current_line = std::begin(lines);
  let p1 = aoc::to_number<u32>(
      aoc::trim(*current_line).substr(sizeof("Player 1 starting position:")));
  ++current_line;
  let p2 = aoc::to_number<u32>(
      aoc::trim(*current_line).substr(sizeof("Player 2 starting position:")));
  return {p1, p2};
}

struct DeterministicDice {
  u32 current{0};
  fn roll() -> u32 {
    current = current % 100 + 1;
    return current;
  }
};

fn solve_case1(Input positions) -> u32 {
  auto [p1, p2] = positions;
  auto score = std::pair<u32, u32>{0, 0};
  auto rolls = u32{};
  auto dice = DeterministicDice{};
  auto roll_dice = [&](u32 pos) -> u32 {
    let rolled = dice.roll() + dice.roll() + dice.roll();
    rolls += 3;
    return (pos + rolled - 1) % 10 + 1;
  };
  loop {
    p1 = roll_dice(p1);
    score.first += p1;
    if (score.first >= 1000) {
      return score.second * rolls;
    }
    p2 = roll_dice(p2);
    score.second += p2;
    if (score.second >= 1000) {
      return score.first * rolls;
    }
  }
}

fn solve_case2(Input positions) -> u64 {
  // TODO: Implement Part 2
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(739785, solve_case1(example));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(805932, solve_case1(input));

  std::println("Part 2");
  // AOC_EXPECT_RESULT(XXX, solve_case2(example));
  // AOC_EXPECT_RESULT(XXX, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
