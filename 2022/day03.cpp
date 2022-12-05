// https://adventofcode.com/2022/day/3

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

#include "../common.h"

template <class T>
using ascii_array = std::array<T, 256>;

constexpr int calc_priority(char value) {
  if (value < 'a') {
    return static_cast<int>(value - 'A') + 27;
  } else {
    return static_cast<int>(value - 'a') + 1;
  }
}

void solve_part1(const std::string& filename) {
  int score = 0;

  readfile_op(filename, [&](std::string_view line) {
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
  });

  std::cout << filename << " -> " << score << std::endl;
}

void solve_part2(const std::string& filename) {
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

  readfile_op(filename, [&](std::string_view line) {
    for (char value : line) {
      occurrences[index][value] = true;
    }
    ++index;
    if (index < 3) {
      // At this point we're just collecting data
      return;
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
  });

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day03.example");
  solve_part1("day03.input");
  std::cout << "Part 2" << std::endl;
  solve_part2("day03.example");
  solve_part2("day03.input");
}
