// https://adventofcode.com/2016/day/12

#include "assembunny.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

using assembunny::Registers;
using assembunny::XOp;

using Input = Vec<XOp>;

auto parse(String const& filename) -> Input {
  return assembunny::transform(assembunny::parse(filename));
}

template <i64 RegCInit>
fn solve_case(Input const& ops) -> i64 {
  auto registers = Registers{0, 0, RegCInit, 0};
  auto counter = i64{};
  while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
    let op = ops[static_cast<usize>(counter)];
    (void)assembunny::exec(op, registers, counter);
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
