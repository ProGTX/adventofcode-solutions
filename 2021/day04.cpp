// https://adventofcode.com/2021/day/4

#include <algorithm>
#include <array>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

class board {
 public:
  using row_t = std::array<int, 5>;
  static constexpr const size_t size = 5 * 5;

  constexpr board() {
    for (auto& marker : markers) {
      marker = false;
    }
  }

  void print() const {
    for (int i = 0; i < 5; ++i) {
      std::cout << "  ";
      for (int j = 0; j < 5; ++j) {
        int index = (i * 5 + j);
        std::cout << std::setw(2) << values[index] << std::setw(1) << "|"
                  << markers[index] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  constexpr auto add(const row_t& row) {
    if (this->is_complete()) {
      if (!std::is_constant_evaluated()) {
        throw std::runtime_error("Cannot add any more rows");
      }
    }
    auto row_begin = std::begin(values) + (5 * num_rows);
    std::ranges::copy(row, row_begin);
    ++num_rows;
    return row_begin;
  }

  constexpr bool mark(int number) {
    int pos = 0;
    for (; pos < size; ++pos) {
      if (values[pos] != number) {
        continue;
      }
      // Hit a single number
      markers[pos] = true;
      break;
    }
    if (pos >= size) {
      // No bingo
      return false;
    }
    return check_bingo(pos);
  }

  constexpr bool is_complete() const { return (num_rows == 5); }

  constexpr int sum_unmarked() const {
    int sum = 0;
    for (int pos = 0; pos < size; ++pos) {
      if (!markers[pos]) {
        sum += values[pos];
      }
    }
    return sum;
  }

  constexpr bool check_bingo(int position) const {
    // Check current row
    {
      int row_num = position / 5;
      const auto begin = std::begin(markers) + row_num * 5;
      bool bingo = std::ranges::all_of(std::ranges::subrange(begin, begin + 5),
                                       std::identity{});
      if (bingo) {
        return true;
      }
    }

    // Check current column
    {
      int column_num = position % 5;
      bool bingo = true;
      for (int pos = column_num; pos < size; pos += 5) {
        if (!markers[pos]) {
          bingo = false;
          break;
        }
      }
      if (bingo) {
        return true;
      }
    }

    return false;
  }

 private:
  std::array<int, size> values;
  std::array<bool, size> markers;
  int num_rows = 0;
};

void solve_part1(const std::string& filename) {
  std::vector<int> bingo_numbers;
  std::vector<board> boards;
  board* current_board_ptr = nullptr;

  readfile_op_header(
      filename,
      [&](std::string_view line) {
        bingo_numbers = split<decltype(bingo_numbers)>(std::string{line}, ',');
      },
      [&](std::string_view line) {
        if (line.empty()) {
          // Create new board
          current_board_ptr = &boards.emplace_back();

          // Don't do anything with the board yet
          return;
        }
        auto row = split<board::row_t>(std::string{line}, ' ');
        current_board_ptr->add(row);
      });

  int winning_number = 0;
  for (auto number : bingo_numbers) {
    for (auto& current_board : boards) {
      bool bingo = current_board.mark(number);
      if (bingo) {
        winning_number = number;
        current_board_ptr = &current_board;
        goto end_loop;
      }
    }
  }
end_loop:

  std::cout << winning_number << ", " << current_board_ptr->sum_unmarked()
            << std::endl;
  std::cout << filename << " -> "
            << (winning_number * current_board_ptr->sum_unmarked())
            << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day04.example");
  solve_part1("day04.input");
  //  std::cout << "Part 2" << std::endl;
  //  solve_part2("day04.example");
  //  solve_part2("day04.input");
}
