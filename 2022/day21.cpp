// https://adventofcode.com/2022/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <functional>
#include <print>

using value_t = aoc::ratio<i64>;
using operation_t = std::function<value_t(value_t, value_t)>;

template <class index_t, class Operation = operation_t>
struct monkey_t {
  index_t index_lhs;
  index_t index_rhs;

  Option<value_t> value_lhs;
  Option<value_t> value_rhs;
  Operation operation;

  fn is_operation_ready() const -> bool {
    return value_lhs.has_value() && value_rhs.has_value();
  }
  fn apply_operation() const -> value_t {
    return operation(value_lhs.value(), value_rhs.value());
  }
};

using parsing_monkey_t = monkey_t<String, char>;
using parsed_monkeys_t = Vec<std::pair<String, parsing_monkey_t>>;
using monkeys_t = Vec<monkey_t<int>>;
using monkey_it = monkeys_t::iterator;

constexpr let value_op = '@';

fn parse(String const& filename) -> parsed_monkeys_t {
  auto parsed_monkeys = parsed_monkeys_t{};
  for (str line : aoc::views::read_lines(filename)) {
    let[name, full_op] = aoc::split_once<String>(line, ':');
    let[lhs, op, rhs] =
        aoc::split<std::array<String, 3>>(full_op.substr(1), ' ');

    if (op.empty()) {
      value_t value = aoc::to_number<int>(lhs);
      parsed_monkeys.emplace_back(name,
                                  parsing_monkey_t{"", "", value, 0, value_op});
      continue;
    }

    parsed_monkeys.emplace_back(name,
                                parsing_monkey_t{lhs, rhs, None, None, op[0]});
  }
  return parsed_monkeys;
}

fn get_value_monkey(value_t lhs, value_t rhs) -> monkey_t<int> {
  return {-1, -1, lhs, rhs, std::plus{}};
}

fn get_yelled_number(monkeys_t& monkeys, int current) -> value_t {
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

fn transform_monkeys(parsed_monkeys_t const& parsed_monkeys)
    -> std::tuple<monkeys_t, int, int> {
  monkeys_t monkeys;
  let begin_it = std::begin(parsed_monkeys);
  int root_index = 0;
  int human_index = 0;
  for (int index = 0; let& [ name, monkey ] : parsed_monkeys) {
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
    let lhs_it = stdr::find_if(parsed_monkeys, [&](let& monkey_pair) {
      return monkey_pair.first == monkey.index_lhs;
    });
    let rhs_it = stdr::find_if(parsed_monkeys, [&](let& monkey_pair) {
      return monkey_pair.first == monkey.index_rhs;
    });
    monkeys.emplace_back(std::distance(begin_it, lhs_it),
                         std::distance(begin_it, rhs_it), monkey.value_lhs,
                         monkey.value_rhs,
                         aoc::get_binary_op<value_t>(monkey.operation));
  }
  return {monkeys, root_index, human_index};
}

fn solve_case1(parsed_monkeys_t const& parsed_monkeys) -> i64 {
  auto [monkeys, root_index, human_index] = transform_monkeys(parsed_monkeys);
  return static_cast<i64>(get_yelled_number(monkeys, root_index));
}

fn solve_case2(parsed_monkeys_t const& parsed_monkeys) -> i64 {
  auto [monkeys, root_index, human_index] = transform_monkeys(parsed_monkeys);

  monkey_it begin_it = std::begin(monkeys);
  let begin_parsed_it = std::begin(parsed_monkeys);
  let get_index = [&](auto it) {
    if constexpr (std::same_as<decltype(it), monkey_it>) {
      return std::distance(begin_it, it);
    } else {
      return std::distance(begin_parsed_it, it);
    }
  };

  monkey_it current_it = begin_it + human_index;
  loop {
    let current_index = get_index(current_it);
    str current_name = parsed_monkeys[current_index].first;

    let parsed_parent_it = stdr::find_if(parsed_monkeys, [&](let& monkey_pair) {
      return (monkey_pair.second.index_lhs == current_name) ||
             (monkey_pair.second.index_rhs == current_name);
    });
    let parent_index = get_index(parsed_parent_it);
    monkey_it parent_it = begin_it + parent_index;

    bool current_was_left = (parent_it->index_lhs == current_index);
    let sibling_index =
        current_was_left ? parent_it->index_rhs : parent_it->index_lhs;
    char parent_op = parsed_parent_it->second.operation;

    current_it->value_lhs = None;
    current_it->value_rhs = None;

    if (parent_index == root_index) {
      *parent_it = get_value_monkey(0, 0);
      current_it->index_lhs = parent_index;
      current_it->index_rhs = sibling_index;
      current_it->operation = std::plus{};
      break;
    }

    // For non-commutative ops where current was on the right:
    //   parent = sibling - current -> current = sibling - parent
    //   parent = sibling / current -> current = sibling / parent
    // Swap operand order and use the original op instead of its inverse.
    bool swap_operands =
        !current_was_left && (parent_op == '-' || parent_op == '/');
    current_it->index_lhs = swap_operands ? sibling_index : parent_index;
    current_it->index_rhs = swap_operands ? parent_index : sibling_index;
    current_it->operation =
        swap_operands ? aoc::get_binary_op<value_t>(parent_op)
                      : aoc::get_inverse_binary_op<value_t>(parent_op);

    current_it = parent_it;
  }

  return static_cast<i64>(get_yelled_number(monkeys, human_index));
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(152, solve_case1(example));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(291425799367130, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(301, solve_case2(example));
  AOC_EXPECT_RESULT(3219579395609, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
