// https://adventofcode.com/2016/day/25

#include "assembunny.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#endif

using assembunny::Registers;
using assembunny::XOp;

using Input = Vec<XOp>;

auto parse(String const& filename) -> Input {
  return assembunny::transform(assembunny::parse(filename));
}

/// The lowest positive value for register a
/// that makes the program transmit a clock signal
fn solve_case1(Input const& ops) -> i64 {
  using OutValues = aoc::static_vector<i64, 8>;
  static_assert((OutValues::capacity() % 2) == 0);
  auto out_values = OutValues{};
  for (let reg_a_init : Range{i64{1}}) {
    out_values.clear();
    auto registers = Registers{reg_a_init, 0, 0, 0};
    auto counter = i64{};
    while ((counter >= 0) && (static_cast<usize>(counter) < ops.size())) {
      let op = ops[static_cast<usize>(counter)];
      if (let out = assembunny::exec(op, registers, counter)) {
        out_values.push_back(*out);
        if (out_values.size() == out_values.capacity()) {
          if (stdr::all_of(out_values | stdv::chunk(2), [](let& pair) {
                return (pair[0] == 0) && (pair[1] == 1);
              })) {
            return reg_a_init;
          }
          break;
        }
      }
    }
  }
  AOC_UNREACHABLE("Every input has an answer");
}

int main() {
  std::println("Part 1");
  let input = parse("day25.input");
  AOC_EXPECT_RESULT(196, solve_case1(input));

  std::println("Part 2");
  aoc::return_incomplete();

  AOC_RETURN_CHECK_RESULT();
}
