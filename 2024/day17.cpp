// https://adventofcode.com/2024/day/17

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const int max_3bit_end = (1 << 3);
constexpr const int max_3bit_mask = max_3bit_end - 1;
constexpr const std::string_view program_prefix = "Program: ";

using registers_t = std::array<int, 3>;
using program_t = std::vector<int>;
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

constexpr auto run_program(std::span<const int> program,
                           registers_t registers) {
  int instruction_ptr = 0;
  results_t results;

  const auto combo = [&](int value) {
    AOC_ASSERT(value < 7, "Invalid combo operand");
    if (value < 4) {
      return value;
    } else {
      return registers[value - 4];
    }
  };

  const auto division = [&]() {
    const auto numerator = registers[0];
    const auto denominator = 1 << combo(program[instruction_ptr + 1]);
    return numerator / denominator;
  };

  while (instruction_ptr < program.size()) {
    AOC_ASSERT(instruction_ptr >= 0, "Invalid instruction pointer");
    const auto op = program[instruction_ptr];
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
        AOC_ASSERT(false, "Invalid instruction");
    }
  }
  AOC_NOT_CONSTEXPR(std::cout << aoc::print_range(registers) << std::endl;);
  return std::pair{results, registers};
}

static_assert(1 == run_program(std::array{2, 6}, {0, 0, 9}).second[1]);
static_assert(std::ranges::equal(
    results_t{0, 1, 2},
    run_program(std::array{5, 0, 5, 1, 5, 4}, {10, 0, 0}).first));
static_assert(std::ranges::equal(
    results_t{4, 2, 5, 6, 7, 7, 7, 7, 3, 1, 0},
    run_program(std::array{0, 1, 5, 4, 3, 0}, {2024, 0, 0}).first));
static_assert(
    0 == run_program(std::array{0, 1, 5, 4, 3, 0}, {2024, 0, 0}).second[0]);
static_assert(26 == run_program(std::array{1, 7}, {0, 29, 0}).second[1]);
static_assert(44354 ==
              run_program(std::array{4, 0}, {0, 2024, 43690}).second[1]);

template <bool>
std::string solve_case(const std::string& filename) {
  program_t program;
  registers_t registers;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    if (line.empty()) {
      continue;
    }
    if (line.find(program_prefix) != std::string::npos) {
      program = aoc::split<program_t>(line.substr(program_prefix.size()), ',');
    } else {
      line = line.substr(program_prefix.size());
      const auto index = static_cast<int>(line[0] - 'A');
      registers[index] = aoc::to_number<int>(line.substr(3));
    }
  }

  auto [results, new_registers] = run_program(program, registers);
  auto result_str = results | std::views::transform([](int value) {
                      return std::string(1, '0' + static_cast<char>(value));
                    }) |
                    std::views::join_with(',') | aoc::ranges::to<std::string>();
  std::cout << filename << " -> " << result_str << std::endl;
  return result_str;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT("4,6,3,5,6,3,5,2,1,0", solve_case<false>("day17.example"));
  AOC_EXPECT_RESULT("1,7,6,5,1,0,5,0,7", solve_case<false>("day17.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day17.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day17.input"));
  AOC_RETURN_CHECK_RESULT();
}
