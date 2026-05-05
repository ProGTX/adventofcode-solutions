// https://adventofcode.com/2021/day/14

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <string>

using Polymer = String;
using Rules = std::array<std::array<char, 26>, 26>;
using Input = std::pair<Polymer, Rules>;

auto parse(String const& filename) -> Input {
  auto polymer = String{};
  auto rules = Rules{};
  auto parsing_rules = false;
  for (str line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_rules = true;
    } else if (parsing_rules) {
      let[pair, insert] = aoc::split_once(line, " -> ");
      rules[pair[0] - 'A'][pair[1] - 'A'] = insert[0];
    } else {
      polymer = String{line};
    }
  }
  return {std::move(polymer), rules};
}

using Pairs = std::array<std::array<usize, 26>, 26>;

fn apply_rules(Pairs const& pairs, Rules const& rules) -> Pairs {
  auto new_pairs = Pairs{};
  for (usize a = 0; a < 26; ++a) {
    for (usize b = 0; b < 26; ++b) {
      let count = pairs[a][b];
      if (count == 0) {
        continue;
      }
      let insert = rules[a][b];
      if (insert == '\0') {
        continue;
      }
      let c = static_cast<usize>(insert - 'A');
      new_pairs[a][c] += count;
      new_pairs[c][b] += count;
    }
  }
  return new_pairs;
}

template <usize STEPS>
fn solve_case(Input const& input) -> usize {
  let & [ polymer, rules ] = input;
  auto pairs = Pairs{};
  for (usize i = 0; i + 1 < polymer.size(); ++i) {
    ++pairs[polymer[i] - 'A'][polymer[i + 1] - 'A'];
  }
  for (usize step = 0; step < STEPS; ++step) {
    pairs = apply_rules(pairs, rules);
  }
  auto counts = std::array<usize, 26>{};
  for (usize a = 0; a < 26; ++a) {
    for (usize b = 0; b < 26; ++b) {
      counts[a] += pairs[a][b];
    }
  }
  counts[polymer.back() - 'A'] += 1;
  let max = *std::ranges::max_element(counts);
  auto nonzero = counts | std::views::filter([](usize c) { return c > 0; });
  let min = *std::ranges::min_element(nonzero);
  return max - min;
}

int main() {
  std::println("Part 1");
  let example = parse("day14.example");
  AOC_EXPECT_RESULT(1588, solve_case<10>(example));
  let input = parse("day14.input");
  AOC_EXPECT_RESULT(2703, solve_case<10>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2188189693529, solve_case<40>(example));
  AOC_EXPECT_RESULT(2984946368465, solve_case<40>(input));

  AOC_RETURN_CHECK_RESULT();
}
