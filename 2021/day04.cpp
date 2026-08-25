// https://adventofcode.com/2021/day/4

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <fstream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#endif

constexpr let board_size = 5uz;

using Numbers = Vec<u32>;
using board = aoc::array_grid<u32, board_size>;

struct game_t {
  Numbers bingo_numbers;
  Vec<board> boards;
};

auto parse(String const& filename) -> game_t {
  auto file = std::ifstream{filename};
  auto bingo_numbers = aoc::split_to_vec<u32>(aoc::read_single_line(file), ',');

  auto boards = Vec<board>{};
  // The numbers of a single board, gathered until the board is complete
  auto data = Numbers{};
  for (str line : aoc::views::read_lines(file, aoc::keep_empty{})) {
    if (line.empty()) {
      if (!data.empty()) {
        boards.emplace_back(data, board_size, board_size);
        data.clear();
      }
      continue;
    }
    let row = aoc::split<board::row_t, true>(line, ' ');
    data.insert(data.end(), row.begin(), row.end());
  }
  if (!data.empty()) {
    boards.emplace_back(data, board_size, board_size);
  }

  return {std::move(bingo_numbers), std::move(boards)};
}

// A marked number is zeroed out,
// so a full row or column adds up to nothing
fn check_bingo(board const& unmarked) -> bool {
  return stdr::any_of(Range{0uz, board_size}, [&](usize start) {
    return (aocr::accumulate(unmarked.row_view(start), u32{}) == 0) ||
           (aocr::accumulate(unmarked.column_view(start), u32{}) == 0);
  });
}

template <bool all_rounds>
fn solve_case(game_t const& game) -> u32 {
  let & [ bingo_numbers, boards ] = game;

  auto winners = aoc::hash_set<usize>{};
  auto last = std::pair{u32{}, 0uz};
  auto unmarked_boards = boards;
  for (let number : bingo_numbers) {
    for (let board_id : aoc::views::indices_of(boards)) {
      let& current_board = boards[board_id];
      auto& unmarked = unmarked_boards[board_id];

      let index = aocr::position(current_board, number);
      if (!index.has_value()) {
        continue;
      }
      unmarked.begin()[static_cast<isize>(*index)] = 0;
      if (!check_bingo(unmarked)) {
        continue;
      } else if (!all_rounds || (winners.insert(board_id).second &&
                                 (winners.size() == boards.size()))) {
        last = {number, board_id};
        goto end_rounds;
      }
    }
  }
end_rounds:

  let[winning_number, winning_board] = last;
  let score =
      winning_number * aocr::accumulate(unmarked_boards[winning_board], u32{});
  return score;
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(4512, (solve_case<false>(example)));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(64084, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1924, (solve_case<true>(example)));
  AOC_EXPECT_RESULT(12833, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
