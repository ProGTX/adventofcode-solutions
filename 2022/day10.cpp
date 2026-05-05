// https://adventofcode.com/2022/day/10

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>
#include <stdexcept>

constexpr let CRT_WIDTH = 40;
constexpr let CRT_HEIGHT = 6;
using Monitor = aoc::array_grid<char, CRT_WIDTH, CRT_HEIGHT>;
using Result = std::pair<int, String>;

enum class Operation {
  noop = 0,
  addx = 1,
};

struct Instruction {
  Operation operation = Operation::noop;
  int value = 0;
};
using Instructions = Vec<Instruction>;

fn parse_instruction(str line) -> Instruction {
  if (line == "noop") {
    return {};
  }
  let[operation, value_str] = aoc::split_once(line, ' ');
  if (operation == "addx") {
    return {Operation::addx, aoc::to_number<int>(value_str)};
  }
  throw std::runtime_error("Invalid instruction " + String{operation});
}

auto parse(String const& filename) -> Instructions {
  return aoc::views::read_lines(filename) |
         std::views::transform(parse_instruction) |
         aoc::ranges::to<Instructions>();
}

fn draw(Monitor& monitor, int cycle, int register_x) {
  let column = (cycle - 1) % CRT_WIDTH;
  let row = ((cycle - 1) / CRT_WIDTH) % CRT_HEIGHT;
  let pixel = (aoc::abs(column - register_x) <= 1) ? '#' : '.';
  monitor.modify(pixel, row, column);
}

fn signal_strength(int cycle, int register_x) -> int {
  if ((cycle == 20) || (((cycle - 20) % 40) == 0)) {
    return cycle * register_x;
  }
  return 0;
}

fn duration(Operation operation) -> int {
  return static_cast<int>(operation) + 1;
}

fn solve_case(Instructions const& instructions) -> Result {
  auto signal = 0;
  auto monitor = Monitor{'.'};
  auto cycle = 1;
  auto register_x = 1;

  let run_cycle = [&] {
    signal += signal_strength(cycle, register_x);
    draw(monitor, cycle, register_x);
    ++cycle;
  };

  for (let instruction : instructions) {
    for (let _ : Range{0, duration(instruction.operation)}) {
      run_cycle();
    }
    if (instruction.operation == Operation::addx) {
      register_x += instruction.value;
    }
  }

  return {signal, String{monitor.data().data(), monitor.size()}};
}

int main() {
  std::println("Part 1");
  let example = parse("day10.example");
  let[example_signal, example_monitor] = solve_case(example);
  AOC_EXPECT_RESULT(13140, example_signal);
  let input = parse("day10.input");
  let[input_signal, input_monitor] = solve_case(input);
  AOC_EXPECT_RESULT(11820, input_signal);

  std::println("Part 2");
  constexpr let expected_example_out = str( //
      "##..##..##..##..##..##..##..##..##..##.."
      "###...###...###...###...###...###...###."
      "####....####....####....####....####...."
      "#####.....#####.....#####.....#####....."
      "######......######......######......####"
      "#######.......#######.......#######.....");
  AOC_EXPECT_RESULT(expected_example_out, example_monitor);
  constexpr let expected_input_out = str( //
      "####.###....##.###..###..#..#..##..#..#."
      "#....#..#....#.#..#.#..#.#.#..#..#.#..#."
      "###..#..#....#.###..#..#.##...#..#.####."
      "#....###.....#.#..#.###..#.#..####.#..#."
      "#....#....#..#.#..#.#.#..#.#..#..#.#..#."
      "####.#.....##..###..#..#.#..#.#..#.#..#.");
  AOC_EXPECT_RESULT(expected_input_out, input_monitor);

  AOC_RETURN_CHECK_RESULT();
}
