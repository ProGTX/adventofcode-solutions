// https://adventofcode.com/2022/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <print>
#include <ranges>
#include <stdexcept>

using Item = u64;

enum class OperationType {
  add,
  multiply,
  square,
};

struct Operation {
  OperationType type = OperationType::add;
  Item value = 0;
};

struct Test {
  Item divisible_by = 1;
  usize on_true = 0;
  usize on_false = 0;
};

struct Monkey {
  Vec<Item> items;
  Operation operation;
  Test test;
};
using Monkeys = Vec<Monkey>;

fn after_split(str line, str delimiter) -> str {
  return aoc::split_once(line, delimiter)[1];
}

fn parse_operation(str line) -> Operation {
  let expression = after_split(line, " = ");
  let parts = aoc::split_to_array<3>(expression, ' ');
  let op = parts[1];
  let rhs = parts[2];
  if (op == "+") {
    return {OperationType::add, aoc::to_number<Item>(rhs)};
  }
  if (op == "*") {
    if (rhs == "old") {
      return {OperationType::square};
    }
    return {OperationType::multiply, aoc::to_number<Item>(rhs)};
  }
  throw std::runtime_error("Invalid operation " + String{op});
}

fn parse_monkey(str block) -> Monkey {
  let lines = aoc::split_to_vec(block, '\n');
  return Monkey{
      .items = aoc::split<Vec<Item>>(after_split(lines[1], ": "), ", "),
      .operation = parse_operation(lines[2]),
      .test =
          Test{
              .divisible_by =
                  aoc::to_number<Item>(after_split(lines[3], "by ")),
              .on_true =
                  aoc::to_number<usize>(after_split(lines[4], "monkey ")),
              .on_false =
                  aoc::to_number<usize>(after_split(lines[5], "monkey ")),
          },
  };
}

auto parse(String const& filename) -> Monkeys {
  return aoc::split(aoc::trim(aoc::read_file(filename)), "\n\n") |
         stdv::transform(parse_monkey) |
         aoc::ranges::to<Monkeys>();
}

fn inspect(Operation operation, Item old) -> Item {
  switch (operation.type) {
    case OperationType::add:
      return old + operation.value;
    case OperationType::multiply:
      return old * operation.value;
    case OperationType::square:
      return old * old;
  }
  AOC_UNREACHABLE("Invalid operation");
}

template <usize num_rounds, Item relief_factor>
fn solve_case(Monkeys const& input) -> Item {
  auto monkeys = input;
  auto supermodulo = std::numeric_limits<Item>::max();
  if constexpr (relief_factor == 1) {
    supermodulo = 1;
    for (let& monkey : monkeys) {
      supermodulo *= monkey.test.divisible_by;
    }
  }
  auto num_inspected = Vec<Item>(monkeys.size(), 0);

  for (let _ : Range{0uz, num_rounds}) {
    for (usize monkey_id = 0; monkey_id < monkeys.size(); ++monkey_id) {
      auto& monkey = monkeys[monkey_id];
      auto items = std::move(monkey.items);
      num_inspected[monkey_id] += items.size();

      for (let item : items) {
        let worry_level =
            inspect(monkey.operation, item % supermodulo) / relief_factor;
        let next_monkey = ((worry_level % monkey.test.divisible_by) == 0)
                              ? monkey.test.on_true
                              : monkey.test.on_false;
        monkeys[next_monkey].items.push_back(worry_level);
      }
    }
  }

  stdr::sort(num_inspected, std::greater{});
  return num_inspected[0] * num_inspected[1];
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(10605, (solve_case<20, 3>(example)));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(98280, (solve_case<20, 3>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2713310158, (solve_case<10000, 1>(example)));
  AOC_EXPECT_RESULT(17673687232, (solve_case<10000, 1>(input)));

  AOC_RETURN_CHECK_RESULT();
}
