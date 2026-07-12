// https://adventofcode.com/2024/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

using namespace std::string_view_literals;
using word_board_t = aoc::char_grid<>;

// Empty character used both as separator and to signal out of bounds
constexpr let empty_char = '?';

constexpr let row_after_padding = 1uz;

// This pads all four sides, so real data starts at row 1/column 1
// instead of row 0/column 0.
auto parse(String const& filename) -> word_board_t {
  return aoc::read_char_grid(filename, {.padding = empty_char}).first;
}

////////////////////////////////////////////////////////////////////////////////
// PART 1

constexpr let xmas_str = "XMAS"sv;
constexpr let xmas_rstr = "SAMX"sv;

fn count_xmas(stdr::input_range auto&& range) -> u32 {
  // Much faster if we convert the range to a string first
  let range_str = range | aoc::collect_string();
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

fn count_xmas_vertical(word_board_t const& board) -> u32 {
  let num_columns = board.num_columns();
  return aoc::ranges::accumulate(
      Range{0uz, num_columns} | stdv::transform([&](usize column) {
        return count_xmas(
            board |
            stdv::drop(board.linear_index(row_after_padding, column)) |
            stdv::stride(num_columns) |
            stdv::take_while(aoc::not_equal_to_value{empty_char}));
      }),
      u32{});
}

fn test_board() -> word_board_t {
  return //
      {String{"????????"
              "?..X...?"
              "?.SAMX.?"
              "?.A..A.?"
              "?XMAS.S?"
              "?.X....?"
              "?......?"
              "????????"},
       8, 8};
}

static_assert(1 == count_xmas_vertical(test_board()));

fn count_xmas_diagonal(word_board_t const& board) -> u32 {
  let num_columns = board.num_columns();
  let num_rows = board.num_rows();
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
  let right_diag_view = [&]() { return stdv::stride(num_columns + 1); };
  let left_diag_view = [&]() { return stdv::stride(num_columns - 1); };
  let stop_at_terminator = [&]() {
    return stdv::take_while(aoc::not_equal_to_value{empty_char});
  };
  // Count diagonals along the top
  // Skip first and last column because they contain empty terminators
  let top_sum = aoc::ranges::accumulate(
      Range{1uz, num_columns - 1} | stdv::transform([&](usize column) {
        let start = board.linear_index(row_after_padding, column);
        return count_xmas(board |
                          stdv::drop(start) |
                          right_diag_view() |
                          stop_at_terminator()) +
               count_xmas(board |
                          stdv::drop(start) |
                          left_diag_view() |
                          stop_at_terminator());
      }),
      u32{});

  // Count right diagonals along the left edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  let left_edge_sum = aoc::ranges::accumulate(
      Range{row_after_padding + 1, num_rows - 1} |
          stdv::transform([&](usize row) {
            return count_xmas(board |
                              stdv::drop(board.linear_index(row, 1)) |
                              right_diag_view() |
                              stop_at_terminator());
          }),
      u32{});

  // Count left diagonals along the right edge
  // Skip the first row because it was already counted in the first loop
  // Skip last row because it contais empty terminators
  let right_edge_column = num_columns - 2;
  let right_edge_sum = aoc::ranges::accumulate(
      Range{row_after_padding + 1, num_rows - 1} |
          stdv::transform([&](usize row) {
            return count_xmas(
                board |
                stdv::drop(board.linear_index(row, right_edge_column)) |
                left_diag_view() |
                stop_at_terminator());
          }),
      u32{});

  return top_sum + left_edge_sum + right_edge_sum;
}

fn solve_case1(word_board_t const& board) -> u32 {
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

constexpr let normalized_str_size = usize{9};

fn normalize_local_grid(str row1, str row2, str row3) -> String {
  AOC_ASSERT(row1.size() == 3, "All three rows must be size 3");
  AOC_ASSERT(row1.size() == row2.size(), "All three rows must be size 3");
  AOC_ASSERT(row2.size() == row3.size(), "All three rows must be size 3");
  String result;
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

constexpr let x_mas_string_storage = []() {
  std::array<std::array<char, normalized_str_size>, 4> result;
  stdr::copy(normalize_local_grid("M.S", //
                                  ".A.", //
                                  "M.S"  //
                                  ),
             std::begin(result[0]));
  stdr::copy(normalize_local_grid("M.M", //
                                  ".A.", //
                                  "S.S"  //
                                  ),
             std::begin(result[1]));
  stdr::copy(normalize_local_grid("S.S", //
                                  ".A.", //
                                  "M.M"  //
                                  ),
             std::begin(result[2]));
  stdr::copy(normalize_local_grid("S.M", //
                                  ".A.", //
                                  "S.M"  //
                                  ),
             std::begin(result[3]));
  return result;
}();

static constexpr std::array x_mas_strings = {
    str{x_mas_string_storage[0].data(), normalized_str_size},
    str{x_mas_string_storage[1].data(), normalized_str_size},
    str{x_mas_string_storage[2].data(), normalized_str_size},
    str{x_mas_string_storage[3].data(), normalized_str_size},
};

static_assert("M S A M S" == x_mas_strings[0]);

// The algorithm is to find each A, collect the surrounding area,
// transform that into a string, and compare against all valid x_mas strings
fn solve_case2(word_board_t const& board) -> u32 {
  let num_rows = board.num_rows();
  let num_columns = board.num_columns();
  let get_substr = [&](usize row, usize column) {
    return str{&board.at(row, column), 3};
  };
  let is_cross = [&](usize row, usize col) {
    if (board.at(row, col) != 'A') {
      return false;
    }
    // Collect area around the A
    let area_str = normalize_local_grid(get_substr(row - 1, col - 1),
                                        get_substr(row, col - 1),
                                        get_substr(row + 1, col - 1));
    return stdr::contains(x_mas_strings, str{area_str});
  };
  // Go through the board, but skip edges to avoid bound checking
  return aoc::ranges::accumulate(
      Range{1uz, num_rows - 1} | stdv::transform([&](usize row) {
        return static_cast<u32>(
            stdr::count_if(Range{1uz, num_columns - 1},
                           [&](usize col) { return is_cross(row, col); }));
      }),
      u32{});
}

////////////////////////////////////////////////////////////////////////////////
// MAIN

int main() {
  std::println("Part 1");
  let example2 = parse("day04.example2");
  AOC_EXPECT_RESULT(4, solve_case1(example2));
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(18, solve_case1(example));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(2517, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(9, solve_case2(example));
  AOC_EXPECT_RESULT(1960, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
