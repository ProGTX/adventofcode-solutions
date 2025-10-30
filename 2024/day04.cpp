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

////////////////////////////////////////////////////////////////////////////////
// PART 1

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
    sum +=
        count_xmas(board |
                   std::views::drop(column) |
                   std::views::stride(num_columns) |
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
    return std::views::stride(num_columns + 1);
  };
  const auto left_diag_view = [&]() {
    return std::views::stride(num_columns - 1);
  };
  const auto stop_at_terminator = [&]() {
    return std::views::take_while(aoc::not_equal_to_value{empty_char});
  };
  int sum = 0;
  // Count diagonals along the top
  // Skip first and last column because they contain empty terminators
  for (int column = 1; column < num_columns - 1; ++column) {
    sum += count_xmas(board |
                      std::views::drop(column) |
                      right_diag_view() |
                      stop_at_terminator());
    sum += count_xmas(board |
                      std::views::drop(column) |
                      left_diag_view() |
                      stop_at_terminator());
  }
  // Count right diagonals along the left edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  for (int row = 1; row < num_rows - 1; ++row) {
    sum += count_xmas(board |
                      std::views::drop(board.linear_index(row, 1)) |
                      right_diag_view() |
                      stop_at_terminator());
  }
  // Count left diagonals along the right edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  for (int column = board.num_columns() - 2, row = 1; row < num_rows - 1;
       ++row) {
    sum += count_xmas(board |
                      std::views::drop(board.linear_index(row, column)) |
                      left_diag_view() |
                      stop_at_terminator());
  }

  return sum;
}

constexpr int count_xmas_board(const word_board_t& board) {
  return count_xmas(board) +
         count_xmas_vertical(board) +
         count_xmas_diagonal(board);
}

////////////////////////////////////////////////////////////////////////////////
// PART 2

constexpr void set_space(char& c) {
  if (c != empty_char) {
    c = ' ';
  }
}

static constexpr std::size_t normalized_str_size = 9;

constexpr std::string normalize_local_grid(std::string_view row1,
                                           std::string_view row2,
                                           std::string_view row3) {
  AOC_ASSERT(row1.size() == 3, "All three rows must be size 3");
  AOC_ASSERT(row1.size() == row2.size(), "All three rows must be size 3");
  AOC_ASSERT(row2.size() == row3.size(), "All three rows must be size 3");
  std::string result;
  result.reserve(normalized_str_size);
  result += row1;
  result += row2;
  result += row3;
  set_space(result[1]);
  set_space(result[3]);
  set_space(result[5]);
  set_space(result[7]);
  return result;
}

static constexpr auto x_mas_string_storage = []() {
  std::array<std::array<char, normalized_str_size>, 4> result;
  std::ranges::copy(normalize_local_grid("M.S", //
                                         ".A.", //
                                         "M.S"  //
                                         ),
                    std::begin(result[0]));
  std::ranges::copy(normalize_local_grid("M.M", //
                                         ".A.", //
                                         "S.S"  //
                                         ),
                    std::begin(result[1]));
  std::ranges::copy(normalize_local_grid("S.S", //
                                         ".A.", //
                                         "M.M"  //
                                         ),
                    std::begin(result[2]));
  std::ranges::copy(normalize_local_grid("S.M", //
                                         ".A.", //
                                         "S.M"  //
                                         ),
                    std::begin(result[3]));
  return result;
}();

static constexpr std::array x_mas_strings = {
    std::string_view{x_mas_string_storage[0].data(), normalized_str_size},
    std::string_view{x_mas_string_storage[1].data(), normalized_str_size},
    std::string_view{x_mas_string_storage[2].data(), normalized_str_size},
    std::string_view{x_mas_string_storage[3].data(), normalized_str_size},
};

static_assert("M S A M S" == x_mas_strings[0]);

// The algorithm is to find each A, collect the surrounding area,
// transform that into a string, and compare against all valid x_mas strings
static constexpr int count_x_mas_crosses(const word_board_t& board) {
  const auto get_substr = [&](int row, int column) {
    return std::string_view{&board.at(row, column), 3};
  };
  int count = 0;
  // Go through the board, but skip edges to avoid bound checking
  for (int row = 1; row < board.num_rows() - 1; ++row) {
    auto it = board.begin_row(row);
    ++it;
    for (int col = 1; col < board.num_columns() - 1; ++col, ++it) {
      if (*it != 'A') {
        continue;
      }
      // Collect area around the A
      auto area_str = normalize_local_grid(get_substr(row - 1, col - 1),
                                           get_substr(row, col - 1),
                                           get_substr(row + 1, col - 1));
      count += static_cast<int>(
          std::ranges::contains(x_mas_strings, std::string_view{area_str}));
    }
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////
// MAIN

template <bool x_mas>
int solve_case(const std::string& filename) {
  word_board_t board;

  for (std::string line : aoc::views::read_lines(filename)) {
    board.add_row(empty_char + std::move(line) + empty_char);
  }
  board.add_row(std::views::repeat(empty_char, board.row_length()));

  int sum = 0;
  if constexpr (x_mas) {
    sum = count_x_mas_crosses(board);
  } else {
    sum = count_xmas_board(board);
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4, solve_case<false>("day04.example2"));
  AOC_EXPECT_RESULT(18, solve_case<false>("day04.example"));
  AOC_EXPECT_RESULT(2517, solve_case<false>("day04.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(9, solve_case<true>("day04.example"));
  AOC_EXPECT_RESULT(1960, solve_case<true>("day04.input"));
  AOC_RETURN_CHECK_RESULT();
}
