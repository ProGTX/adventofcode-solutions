// https://adventofcode.com/2016/day/10

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <functional>
#include <print>
#include <utility>
#endif

/// A value that starts out in a bot
using InputBin = std::pair<u32, usize>;

struct Bot {
  usize low = {};
  usize high = {};
};

/// Outputs are kept in the same id space as the bots, starting here
constexpr usize FIRST_OUTPUT_ID = 1000;
constexpr usize MAX_OUTPUTS = 21;

struct Input {
  Vec<InputBin> input_bins;
  Vec<Bot> bots;
  usize num_outputs = {};
};

fn parse_target(str kind, str id_str, usize& num_outputs) -> usize {
  let id = aoc::to_number<usize>(id_str);
  if (kind == "bot") {
    return id;
  }
  AOC_ASSERT((kind == "output"), "Invalid target");
  num_outputs = std::max(num_outputs, id + 1);
  return id + FIRST_OUTPUT_ID;
}

auto parse(String const& filename) -> Input {
  auto input = Input{};

  for (str line : aoc::views::read_lines(filename)) {
    let words = aoc::split(line, ' ');
    if (words[0] == "value") {
      // value V goes to bot B
      input.input_bins.emplace_back(aoc::to_number<u32>(words[1]),
                                    aoc::to_number<usize>(words[5]));
    } else {
      // bot B gives low to <target> and high to <target>
      let id = aoc::to_number<usize>(words[1]);
      let low = parse_target(words[5], words[6], input.num_outputs);
      let high = parse_target(words[10], words[11], input.num_outputs);
      if (input.bots.size() <= id) {
        input.bots.resize(id + 1);
      }
      input.bots[id] = Bot{low, high};
    }
  }

  return input;
}

template <bool Multiply>
fn solve_case(Input const& input) -> u32 {
  let target = (input.bots.size() <= 3) ? aoc::closed_range<u32>{2, 5}
                                        : aoc::closed_range<u32>{17, 61};

  auto values = Vec<aoc::static_vector<u32, 2>>(input.bots.size());
  for (let& [ value, bot ] : input.input_bins) {
    values[bot].push_back(value);
  }
  auto outputs = aoc::static_vector<u32, MAX_OUTPUTS>(input.num_outputs, 0u);

  auto bot_stack = Vec<usize>{};
  bot_stack.push_back(aoc::ranges::position(values, 2uz, stdr::size).value());

  let give = [&](usize other, u32 value) {
    if (other >= FIRST_OUTPUT_ID) {
      outputs[other - FIRST_OUTPUT_ID] = value;
    } else {
      values[other].push_back(value);
      if (values[other].size() == 2) {
        bot_stack.push_back(other);
      }
    }
  };

  while (!bot_stack.empty()) {
    let bot_id = aoc::pop_stack(bot_stack);
    let sorted = aoc::closed_range{values[bot_id][0], values[bot_id][1]};
    if constexpr (!Multiply) {
      if (sorted == target) {
        return static_cast<u32>(bot_id);
      }
    }
    give(input.bots[bot_id].low, sorted.begin);
    give(input.bots[bot_id].high, sorted.end);
    values[bot_id].clear();
  }

  if constexpr (Multiply) {
    return stdr::fold_left(outputs | stdv::take(3), 1u, std::multiplies{});
  }

  AOC_UNREACHABLE("Bot not found");
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  AOC_EXPECT_RESULT(2, solve_case<false>(example));
  let input = parse("day10.input");
  AOC_EXPECT_RESULT(116, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(30, solve_case<true>(example));
  AOC_EXPECT_RESULT(23903, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
