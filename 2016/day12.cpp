// https://adventofcode.com/2016/day/12

#include "assembunny.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

namespace op = assembunny::op;
using assembunny::Registers;

using Input = Vec<assembunny::Op>;

auto parse(String const& filename) -> Input {
  return assembunny::parse(filename);
}

template <i64 RegCInit>
fn solve_case(Input const& ops) -> i64 {
  auto registers = Registers{0, 0, RegCInit, 0};
  auto counter = i64{};
  while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
    aoc::match(
        ops[static_cast<usize>(counter)], //
        [&](op::Copy const& op) {
          registers[op.to] = assembunny::read(registers, op.from);
        },
        [&](op::Increase const& op) { registers[op.id] += 1; },
        [&](op::Decrease const& op) { registers[op.id] -= 1; },
        [&](op::JumpNotZero const& op) {
          if (assembunny::read(registers, op.condition) != 0) {
            // The jump is relative to this instruction
            counter += assembunny::read(registers, op.offset) - 1;
          }
        },
        [](op::Toggle const&) {
          AOC_UNREACHABLE("Toggling not allowed here");
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
