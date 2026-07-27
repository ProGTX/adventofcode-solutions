// https://adventofcode.com/2024/day/17

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>
#include <span>

constexpr let max_3bit_end = (1 << 3);
constexpr let max_3bit_mask = max_3bit_end - 1;
constexpr let program_prefix = str{"Program: "};

using registers_t = std::array<i32, 3>;
using program_t = Vec<i32>;
using results_t = program_t;

enum opcodes_t {
  adv = 0,
  bxl = 1,
  bst = 2,
  jnz = 3,
  bxc = 4,
  out = 5,
  bdv = 6,
  cdv = 7,
};

struct Input {
  program_t program;
  registers_t registers;
};

auto parse(String const& filename) -> Input {
  auto program = program_t{};
  auto registers = registers_t{};

  for (String line : aoc::views::read_lines(filename)) {
    if (line.empty()) {
      continue;
    }
    if (line.find(program_prefix) != String::npos) {
      program = aoc::split<program_t>(line.substr(program_prefix.size()), ',');
    } else {
      let register_line = str{line}.substr(program_prefix.size());
      let index = static_cast<int>(register_line[0] - 'A');
      registers[index] = aoc::to_number<i32>(register_line.substr(3));
    }
  }

  return {std::move(program), registers};
}

fn run_program(std::span<const i32> program, registers_t registers) {
  int instruction_ptr = 0;
  auto results = results_t{};

  let combo = [&](i32 value) {
    AOC_ASSERT(value < 7, "Invalid combo operand");
    if (value < 4) {
      return value;
    } else {
      return registers[value - 4];
    }
  };

  let division = [&]() {
    let numerator = registers[0];
    let denominator = 1 << combo(program[instruction_ptr + 1]);
    return numerator / denominator;
  };

  while (instruction_ptr < program.size()) {
    AOC_ASSERT(instruction_ptr >= 0, "Invalid instruction pointer");
    let op = program[instruction_ptr];
    switch (op) {
      case adv:
        registers[0] = division();
        instruction_ptr += 2;
        break;
      case bxl:
        registers[1] = registers[1] ^ program[instruction_ptr + 1];
        instruction_ptr += 2;
        break;
      case bst:
        registers[1] = combo(program[instruction_ptr + 1]) & max_3bit_mask;
        instruction_ptr += 2;
        break;
      case jnz:
        if (registers[0] == 0) {
          instruction_ptr += 2;
        } else {
          instruction_ptr = program[instruction_ptr + 1];
        }
        break;
      case bxc:
        registers[1] = registers[1] ^ registers[2];
        instruction_ptr += 2;
        break;
      case out:
        results.push_back(combo(program[instruction_ptr + 1]) & max_3bit_mask);
        instruction_ptr += 2;
        break;
      case bdv:
        registers[1] = division();
        instruction_ptr += 2;
        break;
      case cdv:
        registers[2] = division();
        instruction_ptr += 2;
        break;
      default:
        AOC_UNREACHABLE("Invalid instruction");
    }
  }
  return std::pair{results, registers};
}

static_assert(1 == run_program(std::array{2, 6}, {0, 0, 9}).second[1]);
static_assert(
    stdr::equal(results_t{0, 1, 2},
                run_program(std::array{5, 0, 5, 1, 5, 4}, {10, 0, 0}).first));
static_assert(
    stdr::equal(results_t{4, 2, 5, 6, 7, 7, 7, 7, 3, 1, 0},
                run_program(std::array{0, 1, 5, 4, 3, 0}, {2024, 0, 0}).first));
static_assert(
    0 == run_program(std::array{0, 1, 5, 4, 3, 0}, {2024, 0, 0}).second[0]);
static_assert(26 == run_program(std::array{1, 7}, {0, 29, 0}).second[1]);
static_assert(44354 ==
              run_program(std::array{4, 0}, {0, 2024, 43690}).second[1]);

fn solve_case1(Input const& input) -> String {
  let[results, new_registers] = run_program(input.program, input.registers);
  return results |
         stdv::transform([](i32 value) {
           return String(1, '0' + static_cast<char>(value));
         }) |
         stdv::join_with(',') |
         aoc::collect_string();
}

int main() {
  std::println("Part 1");
  let example = parse("day17.example");
  AOC_EXPECT_RESULT("4,6,3,5,6,3,5,2,1,0", solve_case1(example));
  let input = parse("day17.input");
  AOC_EXPECT_RESULT("1,7,6,5,1,0,5,0,7", solve_case1(input));

  std::println("Part 2");
  aoc::return_incomplete();
  // AOC_EXPECT_RESULT(117440, solve_case2(example));
  // AOC_EXPECT_RESULT(1337, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
