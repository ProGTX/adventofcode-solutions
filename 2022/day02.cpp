// https://adventofcode.com/2022/day/2

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <ranges>
#include <span>
#include <string_view>

auto parse(String const& filename) -> Vec<std::pair<char, char>> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](std::string_view line) {
           auto [a, b] = aoc::split_once(line, ' ');
           return std::pair{a.at(0), b.at(0)};
         }) |
         aoc::collect_vec<std::pair<char, char>>();
}

enum hand {
  // Starts at 0 instead of 1 so we can do modulo 3
  // Need to add 1 to score accordingly
  rock = 0,
  paper = 1,
  scissors = 2,
};

enum outcome {
  lose = 0,
  draw = 3,
  win = 6,
};

fn solve_case1(std::span<const std::pair<char, char>> input) -> int {
  int score = 0;

  aoc::flat_map<char, int> hand_map = {
      {'A', rock}, {'B', paper}, {'C', scissors},
      {'X', rock}, {'Y', paper}, {'Z', scissors},
  };

  for (auto [opponent_char, response_char] : input) {
    let opponent = hand_map.at(opponent_char);
    let response = hand_map.at(response_char);
    // Add 1 because values are lower
    score += response + 1;
    if (opponent == response) {
      score += draw;
      continue;
    }
    if (((response == rock) && (opponent == scissors)) ||
        ((response - opponent) == 1)) {
      score += win;
    } else {
      score += lose;
    }
  }

  return score;
}

fn solve_case2(std::span<const std::pair<char, char>> input) -> int {
  int score = 0;

  aoc::flat_map<char, int> hand_map = {
      {'A', rock}, {'B', paper}, {'C', scissors},
      {'X', lose}, {'Y', draw},  {'Z', win},
  };

  for (auto [opponent_char, outcome_char] : input) {
    let opponent = hand_map.at(opponent_char);
    let outcome = hand_map.at(outcome_char);
    score += outcome;
    if (outcome == draw) {
      // Add 1 because values are lower
      score += opponent + 1;
      continue;
    }
    int response = 0;
    if (outcome == win) {
      response = ((opponent + 1) % 3);
    } else {
      response = ((3 + opponent - 1) % 3);
    }
    // Add 1 because values are lower
    score += response + 1;
  }

  return score;
}

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT(15, solve_case1(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT(14375, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(12, solve_case2(example));
  AOC_EXPECT_RESULT(10274, solve_case2(input));
  AOC_RETURN_CHECK_RESULT();
}
