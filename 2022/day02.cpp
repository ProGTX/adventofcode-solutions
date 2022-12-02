// https://adventofcode.com/2022/day/2

#include <algorithm>
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

#include "../common.h"

enum hand {
  rock = 1,
  paper = 2,
  scissors = 3,
};

void solve_part1(const std::string& filename) {
  int score = 0;

  std::map<char, int> hand_map = {
      {'A', rock}, {'B', paper}, {'C', scissors},
      {'X', rock}, {'Y', paper}, {'Z', scissors},
  };

  readfile_op(filename, [&](std::string_view line) {
    auto [opponentStr, responseStr] =
        split<std::array<std::string, 2>>(std::string{line}, ' ');
    auto opponent = hand_map.at(opponentStr.at(0));
    auto response = hand_map.at(responseStr.at(0));
    score += response;
    if (opponent == response) {
      // Draw
      score += 3;
      return;
    }
    if (((response == rock) && (opponent == scissors)) ||
        ((response - opponent) == 1)) {
      // Win
      score += 6;
    } else {
      // Lose
    }
  });

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day02.example");
  solve_part1("day02.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case("day02.example");
  // solve_case("day02.input");
}
