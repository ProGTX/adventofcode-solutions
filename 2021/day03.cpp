// https://adventofcode.com/2021/day/3

#include "../common/common.h"

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

template <size_t bit_width>
using bits_array = std::array<int, bit_width>;

template <size_t bit_width>
using bits_lines_t = std::vector<bits_array<bit_width>>;

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
int solve_part1(const std::string& filename) {
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

  auto consumption = to_decimal(gamma) * to_decimal(epsilon);
  std::cout << filename << " -> " << consumption << std::endl;
  return consumption;
}

template <size_t bit_width>
int solve_part2(const std::string& filename) {
  bits_lines_t<bit_width> bits_lines;
  readfile_op(filename, [&](std::string_view line) {
    bits_array<bit_width> bits;
    std::ranges::transform(line, std::begin(bits), [&](unsigned char bit_char) {
      return static_cast<int>(bit_char - '0');
    });
    bits_lines.push_back(bits);
  });

  bits_lines_t<bit_width> bits_lines_oxy{bits_lines};
  bits_lines_t<bit_width> bits_lines_co2{std::move(bits_lines)};

  std::array<bits_lines_t<bit_width>, 2> bits_lines_new_oxy;
  std::array<bits_lines_t<bit_width>, 2> bits_lines_new_co2;

  const auto reset_new_bits = [&] {
    for (int i = 0; i < 2; ++i) {
      bits_lines_new_oxy[i].clear();
      bits_lines_new_co2[i].clear();
    }
  };

  for (int bitPos = 0; bitPos < bit_width; ++bitPos) {
    reset_new_bits();
    for (int i = 0; i < 2; ++i) {
      bits_lines_new_oxy[i].reserve(bits_lines_oxy.size());
      bits_lines_new_co2[i].reserve(bits_lines_co2.size());
    }
    for (const auto& bits : bits_lines_oxy) {
      bits_lines_new_oxy[bits[bitPos]].push_back(bits);
    }
    for (const auto& bits : bits_lines_co2) {
      bits_lines_new_co2[bits[bitPos]].push_back(bits);
    }
    if (bits_lines_new_oxy[0].size() > bits_lines_new_oxy[1].size()) {
      std::swap(bits_lines_oxy, bits_lines_new_oxy[0]);
    } else {
      std::swap(bits_lines_oxy, bits_lines_new_oxy[1]);
    }
    if (bits_lines_new_co2[0].size() <= bits_lines_new_co2[1].size()) {
      std::swap(bits_lines_co2, bits_lines_new_co2[0]);
    } else {
      std::swap(bits_lines_co2, bits_lines_new_co2[1]);
    }
  }

  bits_array<bit_width> oxy_generator{bits_lines_oxy[0]};
  bits_array<bit_width> co2_scrubber{bits_lines_co2[0]};

  auto life_support = to_decimal(oxy_generator) * to_decimal(co2_scrubber);
  std::cout << filename << " -> " << life_support << std::endl;
  return life_support;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(198, solve_part1<5>("day03.example"));
  AOC_EXPECT_RESULT(4138664, solve_part1<12>("day03.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(230, solve_part2<5>("day03.example"));
  AOC_EXPECT_RESULT(4273224, solve_part2<12>("day03.input"));
  AOC_RETURN_CHECK_RESULT();
}
