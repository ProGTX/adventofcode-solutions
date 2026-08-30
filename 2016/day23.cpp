// https://adventofcode.com/2016/day/23

#include "assembunny.h"

#ifndef AOC_IMPORT_STD
#include <print>
#include <variant>
#endif

namespace xop = assembunny::xop;
using assembunny::Registers;
using assembunny::Scalar;
using assembunny::XOp;

using Input = Vec<XOp>;

auto parse(String const& filename) -> Input {
  return assembunny::transform(assembunny::parse(filename));
}

fn toggle(XOp const& op) -> XOp {
  return aoc::match(
      op, //
      [](xop::NoOp const&) -> XOp { return xop::NoOp{}; },
      [](xop::Zero const& zero) -> XOp { return xop::ZeroToggled{zero.id}; },
      [](xop::ZeroToggled const& zero) -> XOp { return xop::Zero{zero.id}; },
      // Add replaces Increase and Decrease
      [](xop::Add const& add) -> XOp {
        return xop::Add{add.id, static_cast<Scalar>(-add.value)};
      },
      [](xop::Toggle const& toggle) -> XOp { return xop::Add{toggle.id, 1}; },
      [](xop::JumpNotZero const& jump) -> XOp {
        return xop::Copy{jump.condition, jump.offset};
      },
      [](xop::Copy const& copy) -> XOp {
        return xop::JumpNotZero{copy.from, copy.to};
      },
      [](xop::LoopAdd const&) -> XOp { AOC_UNREACHABLE("Invalid op"); });
}

/// The value in register a once the program halts
template <i64 NUM_EGGS>
fn solve_case(Input const& input) -> i64 {
  auto ops = input;
  auto registers = Registers{NUM_EGGS, 0, 0, 0};
  auto counter = i64{};
  while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
    let op = ops[static_cast<usize>(counter)];
    if (let* toggle_op = std::get_if<xop::Toggle>(&op)) {
      // The target is relative to this instruction,
      // and may be outside the program
      let target = counter + registers[toggle_op->id];
      if ((target >= 0) && (static_cast<usize>(target) < ops.size())) {
        auto& target_op = ops[static_cast<usize>(target)];
        target_op = toggle(target_op);
      }
      counter += 1;
      continue;
    }
    assembunny::exec(op, registers, counter);
  }
  // Register a
  return registers[0];
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(3, solve_case<7>(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(11739, solve_case<7>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(3, solve_case<12>(example));
  AOC_EXPECT_RESULT(479008299, solve_case<12>(input));

  AOC_RETURN_CHECK_RESULT();
}
