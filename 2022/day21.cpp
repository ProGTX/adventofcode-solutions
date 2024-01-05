// https://adventofcode.com/2022/day/21

#include "../common/common.h"
#include "../common/ratio.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_value_t = std::int64_t;
using value_t = aoc::ratio<int_value_t>;
using operation_t = std::function<value_t(value_t, value_t)>;

template <class index_t, class Operation = operation_t>
struct monkey_t {
  index_t index_lhs;
  index_t index_rhs;

  std::optional<value_t> value_lhs;
  std::optional<value_t> value_rhs;
  Operation operation;

  constexpr bool is_operation_ready() const {
    return value_lhs.has_value() && value_rhs.has_value();
  }
  constexpr value_t apply_operation() const {
    return operation(value_lhs.value(), value_rhs.value());
  }
};

using parsing_monkey_t = monkey_t<std::string, char>;
using parsed_monkeys_t = std::vector<std::pair<std::string, parsing_monkey_t>>;
using monkeys_t = std::vector<monkey_t<int>>;
using monkey_it = monkeys_t::iterator;

monkey_t<int> get_value_monkey(value_t lhs, value_t rhs) {
  return {-1, -1, lhs, rhs, std::plus{}};
}

inline constexpr char value_op = '@';

value_t get_yelled_number(monkeys_t& monkeys, int current) {
  auto& current_monkey = monkeys[current];
  if (current_monkey.is_operation_ready()) {
    return current_monkey.apply_operation();
  }
  current_monkey.value_lhs =
      get_yelled_number(monkeys, current_monkey.index_lhs);
  current_monkey.value_rhs =
      get_yelled_number(monkeys, current_monkey.index_rhs);
  return current_monkey.apply_operation();
}

std::tuple<monkeys_t, int, int> transform_monkeys(
    const parsed_monkeys_t& parsed_monkeys) {
  monkeys_t monkeys;
  auto begin_it = std::begin(parsed_monkeys);
  int root_index = 0;
  int human_index = 0;
  for (int index = 0; const auto& [name, monkey] : parsed_monkeys) {
    if (name == "root") {
      root_index = index;
    } else if (name == "humn") {
      human_index = index;
    }
    ++index;

    if (monkey.operation == value_op) {
      monkeys.push_back(
          get_value_monkey(monkey.value_lhs.value(), monkey.value_rhs.value()));
      continue;
    }
    auto lhs_it =
        std::ranges::find_if(parsed_monkeys, [&](const auto& monkey_pair) {
          return monkey_pair.first == monkey.index_lhs;
        });
    auto rhs_it =
        std::ranges::find_if(parsed_monkeys, [&](const auto& monkey_pair) {
          return monkey_pair.first == monkey.index_rhs;
        });
    monkeys.emplace_back(std::distance(begin_it, lhs_it),
                         std::distance(begin_it, rhs_it), monkey.value_lhs,
                         monkey.value_rhs,
                         get_binary_op<value_t>(monkey.operation));
  }
  return {monkeys, root_index, human_index};
}

value_t solve_part1(const parsed_monkeys_t& parsed_monkeys) {
  auto [monkeys, root_index, human_index] = transform_monkeys(parsed_monkeys);
  return get_yelled_number(monkeys, root_index);
}

value_t solve_part2(const parsed_monkeys_t& parsed_monkeys) {
  auto [monkeys, root_index, human_index] = transform_monkeys(parsed_monkeys);

  // Transform the humn branch

  monkey_it begin_it = std::begin(monkeys);
  auto begin_parsed_it = std::begin(parsed_monkeys);
  const auto get_index = [&](auto it) {
    if constexpr (std::same_as<decltype(it), monkey_it>) {
      return std::distance(begin_it, it);
    } else {
      return std::distance(begin_parsed_it, it);
    }
  };

  monkey_it current_it = begin_it + human_index;
  while (true) {
    auto current_index = get_index(current_it);
    std::string_view current_name = parsed_monkeys[current_index].first;

    auto parsed_parent_it =
        std::ranges::find_if(parsed_monkeys, [&](const auto& monkey_pair) {
          return (monkey_pair.second.index_lhs == current_name) ||
                 (monkey_pair.second.index_rhs == current_name);
        });
    auto parent_index = get_index(parsed_parent_it);
    monkey_it parent_it = begin_it + parent_index;

    current_it->index_lhs = parent_index;
    if (parent_it->index_lhs == current_index) {
      current_it->index_rhs = parent_it->index_rhs;
    } else {
      current_it->index_rhs = parent_it->index_lhs;
    }
    current_it->value_lhs = std::nullopt;
    current_it->value_rhs = std::nullopt;

    if (parent_index == root_index) {
      *parent_it = get_value_monkey(0, 0);
      current_it->operation = std::plus{};
      break;
    } else {
      current_it->operation =
          get_inverse_binary_op<value_t>(parsed_parent_it->second.operation);
      // Do not break
    }

    current_it = parent_it;
  }

  return get_yelled_number(monkeys, human_index);
}

template <bool part2>
int_value_t solve_case(const std::string& filename) {
  parsed_monkeys_t parsed_monkeys;
  readfile_op(filename, [&](std::string_view line) {
    auto [name, full_op] = split<std::array<std::string, 2>>(line, ':');
    auto [lhs, op, rhs] = split<std::array<std::string, 3>>(full_op, ' ');

    if (op.empty()) {
      value_t value = to_number<int>(lhs);
      parsed_monkeys.emplace_back(name,
                                  parsing_monkey_t{"", "", value, 0, value_op});
      return;
    }

    parsed_monkeys.emplace_back(
        name, parsing_monkey_t{lhs, rhs, std::nullopt, std::nullopt, op[0]});
  });

  int_value_t number = 0;
  if constexpr (!part2) {
    number = static_cast<int_value_t>(solve_part1(parsed_monkeys));
  } else {
    number = static_cast<int_value_t>(solve_part2(parsed_monkeys));
  }
  std::cout << filename << " -> " << number << std::endl;
  return number;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(152, (solve_case<false>("day21.example")));
  AOC_EXPECT_RESULT(291425799367130, (solve_case<false>("day21.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(301, (solve_case<true>("day21.example")));
  AOC_EXPECT_RESULT(3219579395609, (solve_case<true>("day21.input")));
  AOC_RETURN_CHECK_RESULT();
}
