// https://adventofcode.com/2024/day/4

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

using namespace std::string_view_literals;
using word_board_t = aoc::char_grid<>;

// Empty character used both as separator and to signal out of bounds
static constexpr char empty_char = '?';
static constexpr auto xmas_str = "XMAS"sv;
static constexpr auto xmas_rstr = "SAMX"sv;

constexpr int count_xmas(std::ranges::input_range auto&& range) {
  // Much faster if we convert the range to a string first
  auto range_str = range | aoc::ranges::to<std::string>();
  return aoc::count_substrings(range_str, xmas_str) +
         aoc::count_substrings(range_str, xmas_rstr);
}

static_assert(1 == count_xmas("MMMSXXMASM"sv));
static_assert(1 == count_xmas("MSAMXMSMSA"sv));
static_assert(0 == count_xmas("AMXSXMAAMM"sv));
static_assert(0 == count_xmas("MSAMASMSMX"sv));
static_assert(2 == count_xmas("XMASAMXAMM"sv));
static_assert(0 == count_xmas("XXAMMXXAMA"sv));
static_assert(0 == count_xmas("SMSMSASXSS"sv));
static_assert(0 == count_xmas("SAXAMASAAA"sv));
static_assert(0 == count_xmas("MAMMMXMMMM"sv));
static_assert(1 == count_xmas("MXMXAXMASX"sv));

constexpr int count_xmas_vertical(const word_board_t& board) {
  const auto num_columns = board.num_columns();
  int sum = 0;
  for (int column = 0; column < num_columns; ++column) {
    sum += count_xmas(
        board | std::views::drop(column) | aoc::views::stride(num_columns) |
        std::views::take_while(aoc::not_equal_to_value{empty_char}));
  }
  return sum;
}

constexpr word_board_t test_board() {
  return {std::string{"?..X...?"
                      "?.SAMX.?"
                      "?.A..A.?"
                      "?XMAS.S?"
                      "?.X....?"
                      "?......?"
                      "????????"},
          7, 8};
}

static_assert(1 == count_xmas_vertical(test_board()));

constexpr int count_xmas_diagonal(const word_board_t& board) {
  const auto num_columns = board.num_columns();
  const auto num_rows = board.num_rows();
  AOC_ASSERT(num_columns <= (num_rows + 1),
             "Some simplifications assume a tall board");
  AOC_ASSERT((num_columns >= 3) && (num_rows >= 2),
             "Board needs to be big enough to contain empty terminators");
  AOC_ASSERT((board.at(0, 0) == empty_char) && (board.at(1, 0) == empty_char),
             "Board requires terminators on the left");
  AOC_ASSERT((board.at(0, num_columns - 1) == empty_char) &&
                 (board.at(1, num_columns - 1) == empty_char),
             "Board requires terminators on the right");
  AOC_ASSERT((board.at(num_rows - 1, 0) == empty_char) &&
                 (board.at(num_rows - 1, num_columns - 1) == empty_char) &&
                 (board.at(num_rows - 1, num_columns / 2) == empty_char),
             "Board requires terminators at the bottom");
  const auto right_diag_view = [&]() {
    return aoc::views::stride(num_columns + 1);
  };
  const auto left_diag_view = [&]() {
    return aoc::views::stride(num_columns - 1);
  };
  const auto stop_at_terminator = [&]() {
    return std::views::take_while(aoc::not_equal_to_value{empty_char});
  };
  int sum = 0;
  // Count diagonals along the top
  // Skip first and last column because they contain empty terminators
  for (int column = 1; column < num_columns - 1; ++column) {
    sum += count_xmas(board | std::views::drop(column) | right_diag_view() |
                      stop_at_terminator());
    sum += count_xmas(board | std::views::drop(column) | left_diag_view() |
                      stop_at_terminator());
  }
  // Count right diagonals along the left edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  for (int row = 1; row < num_rows - 1; ++row) {
    sum += count_xmas(board | std::views::drop(board.linear_index(row, 1)) |
                      right_diag_view() | stop_at_terminator());
  }
  // Count left diagonals along the right edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  for (int column = board.num_columns() - 2, row = 1; row < num_rows - 1;
       ++row) {
    sum +=
        count_xmas(board | std::views::drop(board.linear_index(row, column)) |
                   left_diag_view() | stop_at_terminator());
  }

  return sum;
}

constexpr int count_xmas_board(const word_board_t& board) {
  return count_xmas(board) + count_xmas_vertical(board) +
         count_xmas_diagonal(board);
}

template <bool>
int solve_case(const std::string& filename) {
  word_board_t board;

  for (std::string line : aoc::views::read_lines(filename)) {
    board.add_row(empty_char + std::move(line) + empty_char);
  }
  board.add_row(aoc::views::repeat(empty_char, board.row_length()));

  auto sum = count_xmas_board(board);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4, solve_case<false>("day04.example2"));
  AOC_EXPECT_RESULT(18, solve_case<false>("day04.example"));
  AOC_EXPECT_RESULT(2517, solve_case<false>("day04.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(48, solve_case<true>("day04.example2"));
  // AOC_EXPECT_RESULT(-61636489, solve_case<true>("day04.input"));
  AOC_RETURN_CHECK_RESULT();
}
