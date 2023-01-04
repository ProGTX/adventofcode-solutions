// https://adventofcode.com/2022/day/21

#include "../common/common.h"
#include "../common/print.h"

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

using value_t = std::int64_t;
using operation_t = std::function<value_t(value_t, value_t)>;

template <class index_t>
struct monkey_t {
  index_t index_lhs;
  index_t index_rhs;

  std::optional<value_t> value_lhs;
  std::optional<value_t> value_rhs;
  operation_t operation;

  constexpr bool is_operation_ready() const {
    return value_lhs.has_value() && value_rhs.has_value();
  }
  constexpr value_t apply_operation() const {
    return operation(value_lhs.value(), value_rhs.value());
  }
};

using monkeys_t = std::vector<monkey_t<int>>;
using monkey_it = monkeys_t::iterator;

template <bool part2>
value_t get_yelled_number(monkeys_t& monkeys, int current) {
  auto& current_monkey = monkeys[current];
  if (current_monkey.is_operation_ready()) {
    return current_monkey.apply_operation();
  }
  current_monkey.value_lhs =
      get_yelled_number<part2>(monkeys, current_monkey.index_lhs);
  current_monkey.value_rhs =
      get_yelled_number<part2>(monkeys, current_monkey.index_rhs);
  return current_monkey.apply_operation();
}

template <bool part2>
value_t solve_case(const std::string& filename) {
  std::vector<std::pair<std::string, monkey_t<std::string>>> parsed_monkeys;
  readfile_op(filename, [&](std::string_view line) {
    auto [name, full_op] = split<std::array<std::string, 2>>(line, ':');
    auto [lhs, op, rhs] = split<std::array<std::string, 3>>(full_op, ' ');

    if (op.empty()) {
      value_t value = std::stoi(lhs);
      parsed_monkeys.emplace_back(
          name, monkey_t<std::string>{"", "", value, 0, std::plus{}});
      return;
    }

    parsed_monkeys.emplace_back(
        name, monkey_t<std::string>{lhs, rhs, std::nullopt, std::nullopt,
                                    get_binary_op<value_t>(op[0])});
  });

  monkeys_t monkeys;
  auto begin_it = std::begin(parsed_monkeys);
  auto root_index = 0;
  for (int index = 0; const auto& [name, monkey] : parsed_monkeys) {
    if (name == "root") {
      root_index = index;
    }
    ++index;

    if (monkey.index_lhs.empty()) {
      monkeys.emplace_back(-1, -1, monkey.value_lhs, monkey.value_rhs,
                           monkey.operation);
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
                         monkey.value_rhs, monkey.operation);
  }

  value_t final_num = get_yelled_number<part2>(monkeys, root_index);
  std::cout << filename << " -> " << final_num << std::endl;
  return final_num;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(152, (solve_case<false>("day21.example")));
  AOC_EXPECT_RESULT(291425799367130, (solve_case<false>("day21.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(1623178306, (solve_case<true>("day21.example")));
  // AOC_EXPECT_RESULT(14579387544492, (solve_case<true>("day21.input")));
  AOC_RETURN_CHECK_RESULT();
}
