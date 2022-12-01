// https://adventofcode.com/2021/day/3

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

template <size_t bit_width>
using bits_array = std::array<int, bit_width>;

template <size_t bit_width>
int to_decimal(const bits_array<bit_width>& bits) {
  int decimal = 0;
  for (int multiplier = 1; auto bit : bits | std::ranges::views::reverse) {
    decimal += bit * multiplier;
    multiplier *= 2;
  }
  return decimal;
}

template <size_t bit_width>
void solve_part1(const std::string& filename) {
  std::array<std::array<int, 2>, bit_width> occurrences;
  for (int i = 0; i < bit_width; ++i) {
    occurrences[i] = {0, 0};
  }

  readfile_op(filename, [&](std::string_view line) {
    for (int bitPos = 0; auto bit_char : line) {
      auto bit = static_cast<int>(bit_char - '0');
      ++occurrences[bitPos][bit];
      ++bitPos;
    }
  });

  bits_array<bit_width> gamma;
  bits_array<bit_width> epsilon;
  for (int i = 0; i < bit_width; ++i) {
    const bool more_ones = (occurrences[i][1] >= occurrences[i][0]);
    gamma[i] = more_ones ? 1 : 0;
    epsilon[i] = more_ones ? 0 : 1;
  }

  std::cout << filename << " -> " << (to_decimal(gamma) * to_decimal(epsilon))
            << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1<5>("day03.example");
  solve_part1<12>("day03.input");
  // std::cout << "Part 2" << std::endl;
  // solve_part2<5>("day03.example");
  // solve_part2<12>("day03.input");
}
