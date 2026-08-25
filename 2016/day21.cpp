// https://adventofcode.com/2016/day/21

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <ranges>
#include <utility>
#include <variant>
#endif

/// A scrambling operation, taken from the first two words of a line,
/// with the position or letter values it works on
struct SwapPosition {
  usize x;
  usize y;
};
struct SwapLetter {
  char x;
  char y;
};
struct ReversePositions {
  usize from;
  usize to;
};
struct RotateLeft {
  usize steps;
};
struct RotateRight {
  usize steps;
};
struct RotateBasedOnLetterPos {
  char letter;
};
struct MovePosition {
  usize from;
  usize to;
};
using Op = std::variant<SwapPosition, SwapLetter, ReversePositions, RotateLeft,
                        RotateRight, RotateBasedOnLetterPos, MovePosition>;

/// The scrambling operations
using Input = Vec<Op>;

auto parse(String const& filename) -> Input {
  return aocv::read_lines(filename) |
         stdv::transform([](str line) -> Op {
           let words = aoc::split_to_vec<str>(line, ' ');
           let index = [&](usize word) {
             return aoc::to_number<usize>(words[word]);
           };
           let letter = [&](usize word) { return words[word][0]; };
           if (words[0] == "swap") {
             if (words[1] == "position") {
               // swap position X with position Y
               return SwapPosition{index(2), index(5)};
             }
             // swap letter X with letter Y
             AOC_ASSERT((words[1] == "letter"), "Invalid instruction");
             return SwapLetter{letter(2), letter(5)};
           }
           if (words[0] == "reverse") {
             // reverse positions X through Y
             return ReversePositions{index(2), index(4)};
           }
           if (words[0] == "rotate") {
             if (words[1] == "left") {
               // rotate left X steps
               return RotateLeft{index(2)};
             }
             if (words[1] == "right") {
               // rotate right X steps
               return RotateRight{index(2)};
             }
             // rotate based on position of letter X
             AOC_ASSERT((words[1] == "based"), "Invalid instruction");
             return RotateBasedOnLetterPos{letter(6)};
           }
           // move position X to position Y
           AOC_ASSERT((words[0] == "move"), "Invalid instruction");
           return MovePosition{index(2), index(5)};
         }) |
         aoc::collect_vec<Op>();
}

/// The scrambled password, or with `REVERSE`,
/// the password that scrambles into the given one
template <bool REVERSE>
fn solve_case(Input const& instructions, str password_in) -> String {
  auto password = String{password_in};
  let apply = [&](Op const& op) {
    aoc::match(
        op, //
        [&](SwapPosition const& swap) {
          std::swap(password[swap.x], password[swap.y]);
        },
        [&](SwapLetter const& swap) {
          let ix = password.find(swap.x);
          let iy = password.find(swap.y);
          std::swap(password[ix], password[iy]);
        },
        [&](RotateLeft const& rotate) {
          if constexpr (REVERSE) {
            aocr::rotate_right(password, rotate.steps);
          } else {
            aocr::rotate_left(password, rotate.steps);
          }
        },
        [&](RotateRight const& rotate) {
          if constexpr (REVERSE) {
            aocr::rotate_left(password, rotate.steps);
          } else {
            aocr::rotate_right(password, rotate.steps);
          }
        },
        [&](RotateBasedOnLetterPos const& rotate) {
          let len = password.size();
          let ix = password.find(rotate.letter);
          if constexpr (REVERSE) {
            // Which index the letter must have been at
            // for the rotation to have left it where it is now
            let ix_before = *stdr::find_if(Range{0uz, len}, [=](usize before) {
              return (((2 * before) + 1 + static_cast<usize>(before >= 4)) %
                      len) == ix;
            });
            let steps = (ix + len - ix_before) % len;
            aocr::rotate_left(password, steps);
          } else {
            // The rotation can be longer than the password itself
            let steps = (1 + ix + static_cast<usize>(ix >= 4)) % len;
            aocr::rotate_right(password, steps);
          }
        },
        [&](ReversePositions const& reverse) {
          stdr::reverse(password | aocv::slice(reverse.from, reverse.to + 1));
        },
        [&](MovePosition const& move) {
          // Moving a letter back is the same move the other way round
          let[from, to] = REVERSE ? std::pair{move.to, move.from}
                                  : std::pair{move.from, move.to};
          if (from < to) {
            aocr::rotate_left(password | aocv::slice(from, to + 1));
          } else {
            aocr::rotate_right(password | aocv::slice(to, from + 1));
          }
        });
  };
  // Undoing the scrambling means walking the operations backwards,
  // each one undone rather than applied
  if constexpr (REVERSE) {
    for (let& op : instructions | stdv::reverse) {
      apply(op);
    }
  } else {
    for (let& op : instructions) {
      apply(op);
    }
  }
  return password;
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT("decab", solve_case<false>(example, "abcde"));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT("baecdfgh", solve_case<false>(input, "abcdefgh"));

  std::println("Part 2");
  // De-scrambling doesn't work on example input
  AOC_EXPECT_RESULT("abcdefgh", solve_case<true>(input, "baecdfgh"));
  AOC_EXPECT_RESULT("cegdahbf", solve_case<true>(input, "fbgdceah"));

  AOC_RETURN_CHECK_RESULT();
}
