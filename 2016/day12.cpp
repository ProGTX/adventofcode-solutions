// https://adventofcode.com/2016/day/12

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <array>
#include <print>
#include <variant>
#endif

/// Registers a to d, as ids 0 to 3
struct Register {
  usize id;
};
using Registers = std::array<i64, 4>;

/// An instruction's argument: either a literal, or the register holding it
using Value = std::variant<i32, Register>;

struct Copy {
  Value from;
  Register to;
};
struct Increase {
  Register reg;
};
struct Decrease {
  Register reg;
};
struct JumpNotZero {
  Value condition;
  Value offset;
};
using Op = std::variant<Copy, Increase, Decrease, JumpNotZero>;

using Input = Vec<Op>;

fn parse_register(str word) -> Register {
  AOC_ASSERT((word.size() == 1), "Invalid register");
  return Register{static_cast<usize>(word[0] - 'a')};
}

fn parse_value(str word) -> Value {
  if ((word.size() == 1) && (word[0] >= 'a') && (word[0] <= 'd')) {
    return parse_register(word);
  }
  return aoc::to_number<i32>(word);
}

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) -> Op {
           let words = aoc::split_to_vec<str>(line, ' ');
           if (words[0] == "cpy") {
             return Copy{parse_value(words[1]), parse_register(words[2])};
           }
           if (words[0] == "inc") {
             return Increase{parse_register(words[1])};
           }
           if (words[0] == "dec") {
             return Decrease{parse_register(words[1])};
           }
           AOC_ASSERT((words[0] == "jnz"), "Invalid instruction");
           return JumpNotZero{parse_value(words[1]), parse_value(words[2])};
         }) |
         aoc::collect_vec<Op>();
}

fn read(Registers const& registers, Value value) -> i64 {
  return aoc::match(
      value, //
      [](i32 literal) { return static_cast<i64>(literal); },
      [&](Register reg) { return registers[reg.id]; });
}

template <i64 RegCInit>
fn solve_case(Input const& ops) -> i64 {
  auto registers = Registers{0, 0, RegCInit, 0};
  auto counter = i64{};
  while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
    aoc::match(
        ops[static_cast<usize>(counter)], //
        [&](Copy const& op) { registers[op.to.id] = read(registers, op.from); },
        [&](Increase const& op) { registers[op.reg.id] += 1; },
        [&](Decrease const& op) { registers[op.reg.id] -= 1; },
        [&](JumpNotZero const& op) {
          if (read(registers, op.condition) != 0) {
            // The jump is relative to this instruction
            counter += read(registers, op.offset) - 1;
          }
        });
    ++counter;
  }

  // Register a
  return registers[0];
}

int main() {
  std::println("Part 1");
  let example = parse("day12.example");
  AOC_EXPECT_RESULT(42, solve_case<0>(example));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(318083, solve_case<0>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(42, solve_case<1>(example));
  AOC_EXPECT_RESULT(9227737, solve_case<1>(input));

  AOC_RETURN_CHECK_RESULT();
}
