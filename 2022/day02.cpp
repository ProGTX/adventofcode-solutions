// https://adventofcode.com/2022/day/2

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

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

int solve_part1(const std::string& filename) {
  int score = 0;

  std::map<char, int> hand_map = {
      {'A', rock}, {'B', paper}, {'C', scissors},
      {'X', rock}, {'Y', paper}, {'Z', scissors},
  };

  readfile_op(filename, [&](std::string_view line) {
    auto [opponentStr, responseStr] =
        split<std::array<std::string, 2>>(line, ' ');
    auto opponent = hand_map.at(opponentStr.at(0));
    auto response = hand_map.at(responseStr.at(0));
    // Add 1 because values are lower
    score += response + 1;
    if (opponent == response) {
      score += draw;
      return;
    }
    if (((response == rock) && (opponent == scissors)) ||
        ((response - opponent) == 1)) {
      score += win;
    } else {
      score += lose;
    }
  });

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int solve_part2(const std::string& filename) {
  int score = 0;

  std::map<char, int> hand_map = {
      {'A', rock}, {'B', paper}, {'C', scissors},
      {'X', lose}, {'Y', draw},  {'Z', win},
  };

  readfile_op(filename, [&](std::string_view line) {
    auto [opponentStr, outcomeStr] =
        split<std::array<std::string, 2>>(line, ' ');
    auto opponent = hand_map.at(opponentStr.at(0));
    auto outcome = hand_map.at(outcomeStr.at(0));
    score += outcome;
    if (outcome == draw) {
      // Add 1 because values are lower
      score += opponent + 1;
      return;
    }
    int response = 0;
    if (outcome == win) {
      response = ((opponent + 1) % 3);
    } else {
      response = ((3 + opponent - 1) % 3);
    }
    // Add 1 because values are lower
    score += response + 1;
  });

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(15, solve_part1("day02.example"));
  AOC_EXPECT_RESULT(14375, solve_part1("day02.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(12, solve_part2("day02.example"));
  AOC_EXPECT_RESULT(10274, solve_part2("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
