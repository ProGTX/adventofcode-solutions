// https://adventofcode.com/2022/day/3

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

template <class T>
using ascii_array = std::array<T, 256>;

constexpr int calc_priority(char value) {
  if (value < 'a') {
    return static_cast<int>(value - 'A') + 27;
  } else {
    return static_cast<int>(value - 'a') + 1;
  }
}

int solve_part1(const std::string& filename) {
  int score = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    ascii_array<bool> occurs_in_half1{false};
    ascii_array<bool> occurs_in_half2{false};

    auto midsize = line.size() / 2;
    auto half1 = line.substr(0, midsize);
    auto half2 = line.substr(midsize);
    for (int pos = 0; pos < midsize; ++pos) {
      occurs_in_half1[half1.at(pos)] = true;
      occurs_in_half2[half2.at(pos)] = true;
    }
    // Iterate through ASCII table
    for (int pos = static_cast<int>('A'); pos <= static_cast<int>('z'); ++pos) {
      if (occurs_in_half1[pos] && occurs_in_half2[pos]) {
        auto value = static_cast<char>(pos);
        auto priority = calc_priority(value);
        score += priority;
      }
    }
  }

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int solve_part2(const std::string& filename) {
  int score = 0;

  int index = 0;
  std::array<ascii_array<bool>, 3> occurrences;

  const auto reset_count = [&]() {
    // Reset count
    index = 0;
    for (auto& occurrence : occurrences) {
      for (auto& occurs : occurrence) {
        occurs = false;
      }
    }
  };

  reset_count();

  for (std::string_view line : aoc::views::read_lines(filename)) {
    for (char value : line) {
      occurrences[index][value] = true;
    }
    ++index;
    if (index < 3) {
      // At this point we're just collecting data
      continue;
    }

    // Iterate through ASCII table
    for (int pos = static_cast<int>('A'); pos <= static_cast<int>('z'); ++pos) {
      bool occurs_all = true;
      for (const auto& occurrence : occurrences) {
        occurs_all = occurs_all && occurrence[pos];
      }
      if (occurs_all) {
        auto value = static_cast<char>(pos);
        auto priority = calc_priority(value);
        score += priority;
      }
    }

    reset_count();
  }

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(157, solve_part1("day03.example"));
  AOC_EXPECT_RESULT(7793, solve_part1("day03.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(70, solve_part2("day03.example"));
  AOC_EXPECT_RESULT(2499, solve_part2("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
