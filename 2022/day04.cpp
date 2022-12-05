// https://adventofcode.com/2022/day/4

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
#include <utility>
#include <vector>

#include "../common.h"

struct range {
  constexpr range(int begin_ = 1, int end_ = 1) : begin{begin_}, end{end_} {}
  constexpr range(std::array<int, 2> range_array)
      : range{range_array[0], range_array[1]} {}

  constexpr bool contains(range other) const {
    return ((other.begin >= begin) && (other.end <= end));
  }

  constexpr bool overlaps_with(range other) const {
    return ((begin <= other.end) && (end >= other.begin));
  }

  int begin;
  int end;
};

template <bool use_overlaps>
void solve_case(const std::string& filename) {
  int score = 0;

  readfile_op(filename, [&](std::string_view line) {
    auto [firstElfStr, secondElfStr] =
        split<std::array<std::string, 2>>(std::string{line}, ',');

    auto firstElf = range{split<std::array<int, 2>>(firstElfStr, '-')};
    auto secondElf = range{split<std::array<int, 2>>(secondElfStr, '-')};

    if constexpr (use_overlaps) {
      if (firstElf.overlaps_with(secondElf)) {
        score += 1;
      }
    } else {
      if (firstElf.contains(secondElf) || secondElf.contains(firstElf)) {
        score += 1;
      }
    }
  });

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day04.example");
  solve_case<false>("day04.input");
  std::cout << "Part 2" << std::endl;
  solve_case<true>("day04.example");
  solve_case<true>("day04.input");
}
