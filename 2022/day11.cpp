// https://adventofcode.com/2022/day/11

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

#include "../common.h"

struct monkey_t {
  using operation_t = std::function<int(int)>;

  struct test_t {
    int divisible_by;
    int on_false;
    int on_true;
  };
  std::vector<int> items;
  operation_t operation;
  test_t test;
};

template <int num_rounds, int num_average>
void solve_case(const std::string& filename) {
  std::vector<monkey_t> monkeys;
  monkey_t* current = nullptr;

  const auto get_binary_op = [](char op) -> std::function<int(int, int)> {
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
      int value = use_old ? 0 : std::stoi(instructions[5]);
      current->operation = [=](int old) {
        if (use_old) {
          return binary_op(old, old);
        } else {
          return binary_op(old, value);
        }
      };
    } else if (instructions[0] == "Test:") {
      current->test.divisible_by = std::stoi(instructions[3]);
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

  std::vector<int> num_inspected(monkeys.size(), 0);
  for (int round = 0; round < num_rounds; ++round) {
    for (int count = 0; auto& monkey : monkeys) {
      for (auto item : monkey.items) {
        auto worry_level = monkey.operation(item);
        worry_level /= 3;
        auto next_monkey = ((worry_level % monkey.test.divisible_by) == 0)
                               ? monkey.test.on_true
                               : monkey.test.on_false;
        monkeys[next_monkey].items.push_back(worry_level);
      }
      num_inspected[count] += monkey.items.size();
      monkey.items.clear();
      ++count;
    }
  }

  std::ranges::sort(num_inspected, std::greater<>{});

  auto score = std::accumulate(std::begin(num_inspected),
                               std::begin(num_inspected) + num_average, 1,
                               std::multiplies<>{});

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<20, 2>("day11.example");
  solve_case<20, 2>("day11.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day11.example");
  // solve_case<true>("day11.input");
}
