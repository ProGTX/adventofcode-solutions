// https://adventofcode.com/2016/day/6

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <limits>
#include <print>
#endif

using Input = aoc::char_grid<>;

auto parse(String const& filename) -> Input {
  return aoc::read_char_grid(filename);
}

constexpr bool LEAST = false;
constexpr bool MOST = true;

constexpr usize NUM_LETTERS = 26;

template <bool Order>
fn solve_case(Input const& messages) -> String {
  auto message = String{};
  for (let column : aoc::views::indices(messages.num_columns())) {
    auto frequency = std::array<u32, NUM_LETTERS>{};
    for (let letter : messages.column_view(column)) {
      ++frequency[static_cast<usize>(letter - 'a')];
    }
    let chosen = [&] {
      if constexpr (Order == MOST) {
        return stdr::max_element(frequency);
      } else {
        // Letters that never appear are not candidates for the least common one
        stdr::replace(frequency, 0u, std::numeric_limits<u32>::max());
        return stdr::min_element(frequency);
      }
    }();
    message.push_back(
        static_cast<char>('a' + stdr::distance(frequency.begin(), chosen)));
  }
  return message;
}

int main() {
  std::println("Part 1");
  let example = parse("day06.example");
  AOC_EXPECT_RESULT("easter", solve_case<MOST>(example));
  let input = parse("day06.input");
  AOC_EXPECT_RESULT("umejzgdw", solve_case<MOST>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("advent", solve_case<LEAST>(example));
  AOC_EXPECT_RESULT("aovueakv", solve_case<LEAST>(input));

  AOC_RETURN_CHECK_RESULT();
}
