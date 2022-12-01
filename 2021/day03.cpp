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
using bits_lines_t = std::vector<bits_array<bit_width>>;

template <size_t bit_width>
using transposed_lines_t = std::array<std::vector<int>, bit_width>;

template <size_t bit_width>
transposed_lines_t<bit_width> transpose(
    const bits_lines_t<bit_width>& container) {
  transposed_lines_t<bit_width> transposed_lines;
  for (int i = 0; i < bit_width; ++i) {
    transposed_lines[i].reserve(container.size());
  }
  for (const auto& bits : container) {
    for (int i = 0; i < bit_width; ++i) {
      transposed_lines[i].push_back(bits[i]);
    }
  }
  return transposed_lines;
}

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
  int horizontal = 0;
  int depth = 0;

  bits_lines_t<bit_width> bits_lines;

  readfile_op(filename, [&](std::string_view line, int linenum) {
    bits_array<bit_width> bits;
    std::ranges::transform(line, std::begin(bits), [](unsigned char bit_char) {
      return static_cast<int>(bit_char - '0');
    });
    bits_lines.push_back(bits);
  });

  auto transposed_lines = transpose(bits_lines);

  bits_array<bit_width> gamma;
  bits_array<bit_width> epsilon;
  for (int i = 0; const auto& downward_bits : transposed_lines) {
    std::array occurrence{0, 0};
    for (const auto bit : downward_bits) {
      ++occurrence[bit];
    }
    const bool more_ones = (occurrence[1] >= occurrence[0]);
    gamma[i] = more_ones ? 1 : 0;
    epsilon[i] = more_ones ? 0 : 1;
    ++i;
  }

  std::cout << filename << " -> " << (to_decimal(gamma) * to_decimal(epsilon))
            << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "Part 1" << std::endl;
  solve_part1<5>("day03.example");
  solve_part1<12>("day03.input");
  //  std::cout << "Part 2" << std::endl;
  //  solve_part2("day03.example");
  //  solve_part2("day03.input");
}
