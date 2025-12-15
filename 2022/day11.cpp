// https://adventofcode.com/2022/day/11

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using item_t = std::int64_t;

struct monkey_t {
  using operation_t = std::function<item_t(item_t)>;

  struct test_t {
    item_t divisible_by;
    int on_false;
    int on_true;
  };
  std::vector<item_t> items;
  operation_t operation;
  test_t test;
};

template <int num_rounds, item_t relief_factor>
item_t solve_case(const std::string& filename) {
  std::vector<monkey_t> monkeys;
  monkey_t* current = nullptr;
  item_t divisible_max = 1;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto items = aoc::split_to_vec<std::string>(line, ',');
    auto instructions = aoc::split_to_vec<std::string>(items[0], ' ');
    if (instructions[0] == "Monkey") {
      current = &monkeys.emplace_back();
    } else if (instructions[0] == "Starting") {
      // First item has to be handled separately because of comma parsing
      current->items.push_back(std::stoi(instructions[2]));
      std::ranges::transform(
          items | std::views::drop(1), std::back_inserter(current->items),
          [](const std::string& str) { return std::stoi(str); });
    } else if (instructions[0] == "Operation:") {
      auto binary_op = aoc::get_binary_op<item_t>(instructions[4][0]);
      bool use_old = (instructions[5] == "old");
      item_t value = use_old ? 0 : std::stoi(instructions[5]);
      current->operation = [=](item_t old) {
        if (use_old) {
          return binary_op(old, old);
        } else {
          return binary_op(old, value);
        }
      };
    } else if (instructions[0] == "Test:") {
      auto divisible_by = std::stoi(instructions[3]);
      divisible_max *= divisible_by;
      current->test.divisible_by = divisible_by;
    } else if (instructions[0] == "If") {
      if (instructions[1] == "true:") {
        current->test.on_true = std::stoi(instructions[5]);
      } else if (instructions[1] == "false:") {
        current->test.on_false = std::stoi(instructions[5]);
      } else {
        throw std::runtime_error("Invalid if: " + instructions[1]);
      }
    } else {
      throw std::runtime_error("Invalid instruction: " + instructions[0]);
    }
  }

  if constexpr (relief_factor != 1) {
    divisible_max = std::numeric_limits<item_t>::max();
  }

  std::vector<item_t> num_inspected(monkeys.size(), 0);
  for (int round = 0; round < num_rounds; ++round) {
    for (int monkey_id = 0; auto& monkey : monkeys) {
      for (const auto item : monkey.items) {
        const auto worry_level =
            monkey.operation(item % divisible_max) / relief_factor;
        auto next_monkey = ((worry_level % monkey.test.divisible_by) == 0)
                               ? monkey.test.on_true
                               : monkey.test.on_false;
        monkeys[next_monkey].items.push_back(worry_level);
      }
      num_inspected[monkey_id] += monkey.items.size();
      monkey.items.clear();
      ++monkey_id;
    }
  }

  std::ranges::sort(num_inspected, std::greater<>{});

  // Store score as an unsigned long to prevent overflow
  const auto score = num_inspected[0] * num_inspected[1];

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(10605, (solve_case<20, 3>("day11.example")));
  AOC_EXPECT_RESULT(98280, (solve_case<20, 3>("day11.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(2713310158, (solve_case<10000, 1>("day11.example")));
  AOC_EXPECT_RESULT(17673687232, (solve_case<10000, 1>("day11.input")));
  AOC_RETURN_CHECK_RESULT();
}
