// https://adventofcode.com/2021/day/4

#include "../common/common.h"

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

class board : public aoc::array_grid<int, 5> {
 private:
  using base_t = aoc::array_grid<int, 5>;
  constexpr static auto m_size = 5 * 5;

 public:
  constexpr board() {
    for (auto& marker : markers) {
      marker = false;
    }
  }

  void print() const {
    this->print_all([this](std::ostream& out, int row, int column) {
      const auto index = this->linear_index(row, column);
      out << std::setw(2) << m_data[index] << std::setw(1) << "|"
          << markers[index] << " ";
    });
  }

  constexpr auto add(const row_t& row) {
    if (this->is_complete()) {
      AOC_NOT_CONSTEXPR(throw std::runtime_error("Cannot add any more rows"));
    }
    base_t::add_row(row);
  }

  constexpr bool mark(int number) {
    int pos = 0;
    for (; pos < size(); ++pos) {
      if (m_data[pos] != number) {
        continue;
      }
      // Hit a single number
      markers[pos] = true;
      break;
    }
    if (pos >= size()) {
      // No bingo
      return false;
    }
    has_bingo_hit = check_bingo(pos);
    return has_bingo_hit;
  }

  constexpr bool has_bingo() const { return has_bingo_hit; }

  constexpr bool is_complete() const { return (this->num_rows_dynamic() == 5); }

  constexpr int sum_unmarked() const {
    int sum = 0;
    for (int pos = 0; pos < size(); ++pos) {
      if (!markers[pos]) {
        sum += m_data[pos];
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
      for (int pos = column_num; pos < size(); pos += 5) {
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
  std::array<bool, m_size> markers;
  bool has_bingo_hit = false;
};

int solve_case(const std::string& filename, int game_rounds) {
  std::vector<int> bingo_numbers;
  std::vector<board> boards;
  board* current_board_ptr = nullptr;

  aoc::readfile_op_header(
      filename,
      [&](std::string_view line) {
        bingo_numbers = aoc::split<decltype(bingo_numbers)>(line, ',');
      },
      [&](std::string_view line) {
        if (line.empty()) {
          // Create new board
          current_board_ptr = &boards.emplace_back();

          // Don't do anything with the board yet
          return;
        }
        auto row = aoc::split<board::row_t>(line, ' ');
        current_board_ptr->add(row);
      });

  if (game_rounds < 0) {
    game_rounds = boards.size();
  }

  int winning_number = 0;
  int starting_number_pos = 0;
  int boards_remaining = boards.size();
  board winning_board;
  for (int round = 0; round < game_rounds; ++round) {
    // Play as many rounds as needed until boards are exhausted
    for (int number_pos = starting_number_pos;
         number_pos < bingo_numbers.size(); ++number_pos) {
      auto number = bingo_numbers[number_pos];
      bool bingo = false;
      for (auto& current_board : boards) {
        if (current_board.has_bingo()) {
          continue;
        }
        bool current_bingo = current_board.mark(number);
        if (current_bingo) {
          bingo = true;
          --boards_remaining;
          winning_number = number;
          winning_board = current_board;
          if (boards_remaining <= 0) {
            // Exhausted all boards
            goto end_rounds;
          }
        } else {
          // We must continue marking the other boards
        }
      }
      ++starting_number_pos;
      if (bingo) {
        break;
      }
    }
  }
end_rounds:

  auto score = winning_number * winning_board.sum_unmarked();
  std::cout << winning_number << ", " << winning_board.sum_unmarked()
            << std::endl;
  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4512, solve_case("day04.example", 1));
  AOC_EXPECT_RESULT(64084, solve_case("day04.input", 1));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1924, solve_case("day04.example", -1));
  AOC_EXPECT_RESULT(12833, solve_case("day04.input", -1));
  AOC_RETURN_CHECK_RESULT();
}
