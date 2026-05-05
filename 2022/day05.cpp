// https://adventofcode.com/2022/day/5

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>
#include <string_view>

struct crate_move {
  int count;
  int from; // 0-indexed
  int to;   // 0-indexed
};

using Stacks = aoc::static_vector<String, 9>;
using Input = std::pair<Stacks, Vec<crate_move>>;

auto parse(String const& filename) -> Input {
  // There can't be more than 9 crates because that would break the parsing
  // Initially we don't know how many stacks we have,
  // and the parsing gets the crates in reverse order
  std::array<String, 9> temp_stacks{};
  int num_stacks = 0;
  Vec<crate_move> moves;

  // Don't trim spaces, we need them in this example
  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_spaces{})) {
    if (line.at(0) != 'm') {
      // First parse the crates
      if (line.at(1) != '1') {
        // Parsing crates
        for (int i = 0, pos = 1; pos < line.size(); pos += 4, ++i) {
          auto crate = line.at(pos);
          if (crate == ' ') {
            continue;
          }
          temp_stacks[i].push_back(crate);
        }
      } else {
        // Done parsing crate stacks, put them in correct order
        auto stack_numbers = aoc::split_to_vec(line, ' ');
        // -2 because there are multiple spaces between numbers,
        // and there's always a space after the last number
        num_stacks =
            aoc::to_number<int>(stack_numbers[stack_numbers.size() - 2].at(0));
        for (int i = 0; i < num_stacks; ++i) {
          std::ranges::reverse(temp_stacks[i]);
        }
      }
    } else {
      // move N from A to B
      auto [move_str, n_str, from_str, from_pos_str, to_str, to_pos_str] =
          aoc::split_to_array<6>(line, ' ');
      moves.emplace_back(aoc::to_number<int>(n_str),
                         aoc::to_number<int>(from_pos_str) - 1,
                         aoc::to_number<int>(to_pos_str) - 1);
    }
  }

  Stacks stacks;
  for (int i = 0; i < num_stacks; ++i) {
    stacks.push_back(std::move(temp_stacks[i]));
  }
  return {stacks, std::move(moves)};
}

template <bool grab_multiple>
fn solve_case(Input const& input) -> String {
  auto [stacks, moves] = input;

  for (auto [count, from, to] : moves) {
    auto& from_crate = stacks[from];
    auto& to_crate = stacks[to];

    if constexpr (!grab_multiple) {
      // Part 1
      for (int i = 0; i < count; ++i) {
        auto crate = from_crate.back();
        to_crate.push_back(crate);
        from_crate.resize(from_crate.size() - 1);
      }
    } else {
      // Part 2
      auto from_crate_new_size = from_crate.size() - count;
      std::string_view crate_bunch{from_crate.data() + from_crate_new_size};
      to_crate += crate_bunch;
      from_crate.resize(from_crate_new_size);
    }
  }

  auto top_stacks = String(stacks.size(), ' ');
  for (int pos = 0; let& stack : stacks) {
    if (!stack.empty()) {
      top_stacks[pos] = stack.back();
      ++pos;
    }
  }
  return top_stacks;
}

int main() {
  std::println("Part 1");
  let example = parse("day05.example");
  AOC_EXPECT_RESULT("CMZ", solve_case<false>(example));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT("QGTHFZBHV", solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("MCD", solve_case<true>(example));
  AOC_EXPECT_RESULT("MGDMPSZTM", solve_case<true>(input));
  AOC_RETURN_CHECK_RESULT();
}
