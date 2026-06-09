// https://adventofcode.com/2022/day/3

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

auto parse(String const& filename) -> Vec<String> {
  return aoc::read_lines(filename);
}

template <class T>
using ascii_array = std::array<T, 256>;

constexpr int calc_priority(char value) {
  if (value < 'a') {
    return static_cast<int>(value - 'A') + 27;
  } else {
    return static_cast<int>(value - 'a') + 1;
  }
}

fn solve_case1(std::span<const String> input) -> int {
  int score = 0;

  for (std::string_view line : input) {
    ascii_array<bool> occurs_in_half1{false};
    ascii_array<bool> occurs_in_half2{false};

    auto midsize = line.size() / 2;
    auto half1 = line.substr(0, midsize);
    auto half2 = line.substr(midsize);
    for (int pos = 0; pos < (int)midsize; ++pos) {
      occurs_in_half1[half1.at(pos)] = true;
      occurs_in_half2[half2.at(pos)] = true;
    }
    // Iterate through ASCII table
    for (int pos = static_cast<int>('A'); pos <= static_cast<int>('z'); ++pos) {
      if (occurs_in_half1[pos] && occurs_in_half2[pos]) {
        score += calc_priority(static_cast<char>(pos));
      }
    }
  }

  return score;
}

fn solve_case2(std::span<const String> input) -> int {
  int score = 0;

  int index = 0;
  std::array<ascii_array<bool>, 3> occurrences;

  const auto reset_count = [&]() {
    index = 0;
    for (auto& occurrence : occurrences) {
      for (auto& occurs : occurrence) {
        occurs = false;
      }
    }
  };

  reset_count();

  for (std::string_view line : input) {
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
        score += calc_priority(static_cast<char>(pos));
      }
    }
    reset_count();
  }

  return score;
}

int main() {
  std::println("Part 1");
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(157, solve_case1(example));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(7793, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(70, solve_case2(example));
  AOC_EXPECT_RESULT(2499, solve_case2(input));
  AOC_RETURN_CHECK_RESULT();
}
