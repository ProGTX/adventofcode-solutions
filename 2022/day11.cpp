// https://adventofcode.com/2022/day/11

#include "../common/common.h"
#include "../common/print.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using item_t = long;

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

  std::ostream& print(std::string_view indent = "",
                      std::ostream& out = std::cout) const {
    out << indent;
    print_range(items, ",", out) << std::endl;
    out << indent << "operation(7) = " << operation(7) << std::endl;
    out << indent << "test{" << test.divisible_by << ",true(" << test.on_true
        << "),"
        << "false(" << test.on_false << ")}" << std::endl;
    return out;
  }
};

template <int num_rounds, int num_average, item_t relief_factor>
void solve_case(const std::string& filename) {
  std::vector<monkey_t> monkeys;
  monkey_t* current = nullptr;
  item_t divisible_max = 1;

  const auto get_binary_op =
      [](char op) -> std::function<item_t(item_t, item_t)> {
    switch (op) {
      case '+':
        return std::plus<>{};
      case '*':
        return std::multiplies<>{};
      default:
        throw std::runtime_error("Invalid operation " + std::string{op});
    }
  };

  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    auto items = split<std::vector<std::string>>(std::string{line}, ',');
    auto instructions = split<std::vector<std::string>>(items[0], ' ');
    if (instructions[0] == "Monkey") {
      current = &monkeys.emplace_back();
    } else if (instructions[0] == "Starting") {
      // First item has to be handled separately because of comma parsing
      current->items.push_back(std::stoi(instructions[2]));
      std::ranges::transform(
          items | std::views::drop(1), std::back_inserter(current->items),
          [](const std::string& str) { return std::stoi(str); });
    } else if (instructions[0] == "Operation:") {
      auto binary_op = get_binary_op(instructions[4][0]);
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
  });

  std::vector<item_t> num_inspected(monkeys.size(), 0);
  for (int round = 0; round < num_rounds; ++round) {
    for (int monkey_id = 0; auto& monkey : monkeys) {
      for (const auto item : monkey.items) {
        auto worry_level = monkey.operation(item);
        if (relief_factor == 1) {
          worry_level = worry_level % divisible_max;
        } else {
          worry_level /= relief_factor;
        }
        auto next_monkey = ((worry_level % monkey.test.divisible_by) == 0)
                               ? monkey.test.on_true
                               : monkey.test.on_false;
        monkeys[next_monkey].items.push_back(worry_level);
      }
      // std::cout << std::endl;
      num_inspected[monkey_id] += monkey.items.size();
      monkey.items.clear();
      ++monkey_id;
    }
  }

  std::ranges::sort(num_inspected, std::greater<>{});

  // Store score as an unsigned long to prevent overflow
  auto score = std::accumulate(std::begin(num_inspected),
                               std::begin(num_inspected) + num_average, 1ul,
                               std::multiplies<>{});

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<20, 2, 3>("day11.example");
  solve_case<20, 2, 3>("day11.input");
  std::cout << "Part 2" << std::endl;
  solve_case<10000, 2, 1>("day11.example");
  solve_case<10000, 2, 1>("day11.input");
}
