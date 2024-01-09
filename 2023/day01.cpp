// https://adventofcode.com/2023/day/1

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

static constexpr auto numbers_view =
    std::views::iota('0', static_cast<char>('9' + 1));

constexpr int find_number(std::string_view str) {
  auto first_it = std::ranges::find_first_of(str, numbers_view);
  auto last_it =
      std::ranges::find_first_of(str | std::views::reverse, numbers_view);
  int number =
      static_cast<int>(*first_it - '0') * 10 + static_cast<int>(*last_it - '0');
  return number;
}

static constexpr std::array<std::string_view, 20> words = {
    "0",    "1",    "2",   "3",     "4",     "5",    "6",
    "7",    "8",    "9",   "zero",  "one",   "two",  "three",
    "four", "five", "six", "seven", "eight", "nine",
};

struct word_it_t {
  size_t str_pos;
  decltype(words)::const_iterator words_it;
};

constexpr int find_number_from_words(std::string_view str) {
  word_it_t first_it{std::size(str), std::end(words)};
  word_it_t last_it{0, std::end(words)};

  for (auto it = std::begin(words); it != std::end(words); ++it) {
    const auto str_pos = str.find(*it);
    if (str_pos == std::string::npos) {
      continue;
    }
    if (str_pos < first_it.str_pos) {
      first_it.str_pos = str_pos;
      first_it.words_it = it;
    }
    const auto str_rpos = str.rfind(*it);
    if ((str_rpos != std::string::npos) && (str_rpos >= last_it.str_pos)) {
      last_it.str_pos = str_rpos;
      last_it.words_it = it;
    }
  }

  int number = static_cast<int>(
      (std::distance(std::begin(words), first_it.words_it) % 10) * 10 +
      (std::distance(std::begin(words), last_it.words_it) % 10));
  return number;
}

static_assert(find_number_from_words("5three37tfnkz") == 57);
static_assert(find_number_from_words("57eight9fivefiveeight") == 58);

template <bool words>
int solve_case(const std::string& filename) {
  int sum = 0;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    if constexpr (words) {
      sum += find_number_from_words(line);
    } else {
      sum += find_number(line);
    }
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(142, solve_case<false>("day01.example"));
  AOC_EXPECT_RESULT(54388, solve_case<false>("day01.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(281, solve_case<true>("day01.example2"));
  AOC_EXPECT_RESULT(53515, solve_case<true>("day01.input"));
  AOC_RETURN_CHECK_RESULT();
}
