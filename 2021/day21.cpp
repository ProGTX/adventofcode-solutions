// https://adventofcode.com/2021/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <vector>

namespace stdv = std::views;

using Input = std::pair<u32, u32>;

fn parse(String const& filename) -> Input {
  auto lines = aoc::views::read_lines(filename);
  auto it = std::begin(lines);
  let p1 = aoc::to_number<u32>(
      aoc::trim(*it).substr(sizeof("Player 1 starting position:")));
  ++it;
  let p2 = aoc::to_number<u32>(
      aoc::trim(*it).substr(sizeof("Player 2 starting position:")));
  return {p1, p2};
}

static constexpr u32 WIN_SCORE_PART1 = 1000;
static constexpr u32 WIN_SCORE_PART2 = 21;
static constexpr usize DIRAC_FREQ_OFFSET = 3;

fn next_pos(u32 pos, u32 roll) -> u32 { return (pos + roll - 1) % 10 + 1; }

struct DeterministicDice {
  u32 current{0};
  fn roll() -> u32 {
    current = current % 100 + 1;
    return current;
  }
};

fn solve_case1(Input positions) -> u32 {
  auto [p1, p2] = positions;
  auto score = std::pair<u32, u32>{0, 0};
  auto rolls = u32{};
  auto dice = DeterministicDice{};
  auto roll_dice = [&](u32 pos) -> u32 {
    let rolled = dice.roll() + dice.roll() + dice.roll();
    rolls += 3;
    return next_pos(pos, rolled);
  };
  loop {
    p1 = roll_dice(p1);
    score.first += p1;
    if (score.first >= WIN_SCORE_PART1) {
      return score.second * rolls;
    }
    p2 = roll_dice(p2);
    score.second += p2;
    if (score.second >= WIN_SCORE_PART1) {
      return score.first * rolls;
    }
  }
}

fn dirac_roll_frequency() -> std::array<u32, 7> {
  auto freq = std::array<u32, 7>{};
  for (u32 a = 1; a <= 3; ++a) {
    for (u32 b = 1; b <= 3; ++b) {
      for (u32 c = 1; c <= 3; ++c) {
        freq[a + b + c - DIRAC_FREQ_OFFSET] += 1;
      }
    }
  }
  return freq;
}

struct Universe {
  std::pair<u32, u32> positions;
  std::pair<u32, u32> score;
  bool is_player_one;

  fn operator<=>(Universe const&) const = default;
};

fn solve_case2(Input positions) -> u64 {
  using Multiverse = std::map<Universe, usize>;
  let dirac = dirac_roll_frequency();

  auto multiverse = Multiverse{};
  multiverse[Universe{positions, {0, 0}, true}] = 1;

  auto num_wins = std::pair<u64, u64>{0, 0};

  while (!multiverse.empty()) {
    auto new_multiverse = Multiverse{};
    for (let& [ universe, uni_count ] : multiverse) {
      for (let[roll_idx, freq] : dirac | stdv::enumerate) {
        let roll = static_cast<u32>(roll_idx + DIRAC_FREQ_OFFSET);
        let new_uni_count = uni_count * static_cast<usize>(freq);

        auto new_universe = universe;
        new_universe.is_player_one = !universe.is_player_one;

        if (universe.is_player_one) {
          // Player 1 rolled
          new_universe.positions.first =
              next_pos(universe.positions.first, roll);
          new_universe.score.first += new_universe.positions.first;

          if (new_universe.score.first >= WIN_SCORE_PART2) {
            num_wins.first += new_uni_count;
            continue;
          }
        } else {
          // Player 2 rolled
          new_universe.positions.second =
              next_pos(universe.positions.second, roll);
          new_universe.score.second += new_universe.positions.second;

          if (new_universe.score.second >= WIN_SCORE_PART2) {
            num_wins.second += new_uni_count;
            continue;
          }
        }

        // Nobody won, explore this universe further
        new_multiverse[new_universe] += new_uni_count;
      }
    }
    multiverse = std::move(new_multiverse);
  }

  return std::max(num_wins.first, num_wins.second);
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(739785, solve_case1(example));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(805932, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(444356092776315ULL, solve_case2(example));
  AOC_EXPECT_RESULT(133029050096658ULL, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
