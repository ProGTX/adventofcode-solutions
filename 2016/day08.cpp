// https://adventofcode.com/2016/day/8

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <string>
#endif

enum class Op {
  Rect,
  RotateRow,
  RotateColumn,
};

struct Instruction {
  Op op;
  usize row;
  usize column;
};

using Input = Vec<Instruction>;

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let[op, rest] = aoc::split_once(line, ' ');
           if (op == "rect") {
             let[columns, rows] = aoc::split_once<usize>(rest, 'x');
             return Instruction{Op::Rect, rows, columns};
           }
           AOC_ASSERT(op == "rotate", "Invalid operation");
           let parts = aoc::split_to_array<4>(rest, ' ');
           let a = aoc::to_number<usize>(parts[1].substr(2));
           let b = aoc::to_number<usize>(parts[3]);
           if (parts[0] == "row") {
             return Instruction{Op::RotateRow, a, b};
           }
           AOC_ASSERT(parts[0] == "column", "Invalid rotation");
           return Instruction{Op::RotateColumn, b, a};
         }) |
         aoc::collect_vec<Instruction>();
}

template <bool Print>
fn solve_case(Input const& instructions) -> String {
  constexpr char LIT = '#';
  constexpr let NUM_ROWS = 6uz;
  constexpr let NUM_COLUMNS = 50uz;
  auto screen = aoc::char_grid<>{' ', NUM_ROWS, NUM_COLUMNS};

  for (let& instruction : instructions) {
    switch (instruction.op) {
      case Op::Rect:
        for (let row : aoc::views::indices(instruction.row)) {
          stdr::fill_n(screen.begin_row(row), instruction.column, LIT);
        }
        break;
      case Op::RotateRow:
        aocr::rotate_right(screen.begin_row(instruction.row),
                           screen.end_row(instruction.row), instruction.column);
        break;
      case Op::RotateColumn: {
        auto column = screen.column_view(instruction.column);
        let first = stdr::begin(column);
        aocr::rotate_right(first, first + NUM_ROWS, instruction.row);
        break;
      }
    }
  }

  if constexpr (!Print) {
    return std::to_string(stdr::count(screen.data(), LIT));
  } else {
    return screen.data();
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT("6", solve_case<false>(example));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT("119", solve_case<false>(input));

  std::println("Part 2");

  AOC_EXPECT_RESULT(("    # #                                           "
                     "# #                                               "
                     " #                                                "
                     " #                                                "
                     "                                                  "
                     "                                                  "),
                    solve_case<true>(example));

  AOC_EXPECT_RESULT(("#### #### #  # ####  ### ####  ##   ##  ###   ##  "
                     "   # #    #  # #    #    #    #  # #  # #  # #  # "
                     "  #  ###  #### ###  #    ###  #  # #    #  # #  # "
                     " #   #    #  # #     ##  #    #  # # ## ###  #  # "
                     "#    #    #  # #       # #    #  # #  # #    #  # "
                     "#### #    #  # #    ###  #     ##   ### #     ##  "),
                    solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
