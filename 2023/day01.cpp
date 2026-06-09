// https://adventofcode.com/2023/day/1

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <span>

auto parse(String const& filename) -> Vec<String> {
  return aoc::views::read_lines(filename) |
         aoc::views::transform_cast<String>() |
         aoc::ranges::to<Vec<String>>();
}

static constexpr auto numbers_view =
    stdv::iota('0', static_cast<char>('9' + 1));

fn find_number(str line) {
  auto first_it = stdr::find_first_of(line, numbers_view);
  auto last_it = stdr::find_first_of(line | stdv::reverse, numbers_view);
  int number =
      static_cast<int>(*first_it - '0') * 10 + static_cast<int>(*last_it - '0');
  return number;
}

static constexpr let words = std::array<str, 20>{
    "0",    "1",    "2",   "3",     "4",     "5",    "6",
    "7",    "8",    "9",   "zero",  "one",   "two",  "three",
    "four", "five", "six", "seven", "eight", "nine",
};

struct word_it_t {
  usize str_pos;
  usize word_index;
};

fn find_number_from_words(str line) -> i32 {
  auto first = word_it_t{line.size(), words.size()};
  auto last = word_it_t{0, words.size()};

  for (let word_index : Range{0uz, words.size()}) {
    let word = words[word_index];
    let str_pos = line.find(word);
    if (str_pos == str::npos) {
      continue;
    }
    if (str_pos < first.str_pos) {
      first = {str_pos, word_index};
    }
    let str_rpos = line.rfind(word);
    if (str_rpos >= last.str_pos) {
      last = {str_rpos, word_index};
    }
  }

  return static_cast<i32>(first.word_index % 10) * 10 +
         static_cast<i32>(last.word_index % 10);
}

static_assert(find_number_from_words("5three37tfnkz") == 57);
static_assert(find_number_from_words("57eight9fivefiveeight") == 58);

fn solve_case1(std::span<const String> lines) -> i32 {
  return aoc::ranges::accumulate(lines | stdv::transform(find_number), 0);
}

fn solve_case2(std::span<const String> lines) -> i32 {
  return aoc::ranges::accumulate(
      lines | stdv::transform(find_number_from_words), 0);
}

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(142, solve_case1(example));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(54388, solve_case1(input));

  std::println("Part 2");
  let example2 = parse("day01.example2");
  AOC_EXPECT_RESULT(281, solve_case2(example2));
  AOC_EXPECT_RESULT(53515, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
