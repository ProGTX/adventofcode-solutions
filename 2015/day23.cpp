// https://adventofcode.com/2015/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

struct Instruction {
  String inst;
  usize reg_index;
  i8 operand;
};

using Instructions = Vec<Instruction>;

auto parse(String const& filename) -> Instructions {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           let[instruction_str, args] = aoc::split_once(line, ' ');
           auto instruction = Instruction{
               .inst = String{instruction_str},
               .reg_index = 0,
               .operand = 0,
           };
           let[first, second] = aoc::split_once(args, ", ");
           if (!second.empty()) {
             if (first == "a") {
               instruction.reg_index = 0;
             } else if (first == "b") {
               instruction.reg_index = 1;
             } else {
               AOC_UNREACHABLE("Invalid first argument");
             }
             instruction.operand = aoc::to_number<i8>(second);
           } else {
             if (first == "a") {
               instruction.reg_index = 0;
             } else if (first == "b") {
               instruction.reg_index = 1;
             } else {
               instruction.operand = aoc::to_number<i8>(first);
             }
           }
           return instruction;
         }) |
         aoc::ranges::to<Instructions>();
}

template <u32 START_A>
fn solve_case(Instructions const& instructions) -> u32 {
  auto registers = std::array{START_A, 0u};
  auto pc = 0uz;
  let jump_if = [&](bool cond, const i8 operand) {
    if (cond) {
      pc = static_cast<usize>((static_cast<i8>(pc) + operand));
    } else {
      pc += 1;
    }
  };
  while (pc < instructions.size()) {
    let & [ inst, reg_index, operand ] = instructions[pc];
    auto& r = registers[reg_index];
    if (inst == "hlf") {
      r /= 2;
      pc += 1;
    } else if (inst == "tpl") {
      r *= 3;
      pc += 1;
    } else if (inst == "inc") {
      r += 1;
      pc += 1;
    } else if (inst == "jmp") {
      jump_if(true, operand);
    } else if (inst == "jie") {
      jump_if((r % 2) == 0, operand);
    } else if (inst == "jio") {
      jump_if(r == 1, operand);
    } else {
      AOC_UNREACHABLE("Invalid operation");
    }
  }
  return registers[1];
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(2, solve_case<0>(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(255, solve_case<0>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2, solve_case<1>(example));
  AOC_EXPECT_RESULT(334, solve_case<1>(input));

  AOC_RETURN_CHECK_RESULT();
}
