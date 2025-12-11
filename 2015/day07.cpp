// https://adventofcode.com/2015/day/7

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>

// A signal can be a wire ID or an an actual value
// We unify them by storing values as 16-bit numbers,
// and the wire ID as numbers that are higher than all signal values
using signal_t = std::uint32_t;
using wire_id_t = signal_t;
using strict_signal_t = std::uint16_t;
constexpr const wire_id_t first_wire_id = 1u << 16;
constexpr bool is_value(signal_t signal) { return signal < first_wire_id; }
constexpr size_t input_id(wire_id_t wire_id) {
  AOC_ASSERT(wire_id >= first_wire_id, "Invalid wire ID");
  return wire_id - first_wire_id;
}
constexpr strict_signal_t strict(signal_t signal) {
  AOC_ASSERT(is_value(signal), "Cannot convert provided signal to value");
  return static_cast<strict_signal_t>(signal);
}

enum class operation_t {
  value_op,
  alias_op,
  not_op,
  and_op,
  or_op,
  lshift_op,
  rshift_op,
};

struct gate_t {
  operation_t op{operation_t::value_op};
  signal_t lhs{0};
  signal_t rhs{0};
};

// Include spaces to avoid trimming later
using namespace std::string_view_literals;
static constexpr const auto arrow_str = " -> "sv;
static constexpr const auto not_str = "NOT "sv;
// Don't include spaces here because we've already trimmed spaces
static constexpr const auto and_str = "AND"sv;
static constexpr const auto or_str = "OR"sv;
static constexpr const auto lshift_str = "LSHIFT"sv;
static constexpr const auto rshift_str = "RSHIFT"sv;

std::tuple<std::vector<gate_t>, signal_t, signal_t> parse(
    const std::string& filename) {
  auto name_to_id = aoc::name_to_id(first_wire_id);
  const auto get_signal = [&](std::string_view wire_name) -> signal_t {
    if (std::isdigit(wire_name[0])) {
      return aoc::to_number<signal_t>(wire_name);
    }
    return name_to_id.intern(wire_name);
  };
  std::vector<gate_t> inputs;
  for (std::string_view line : aoc::views::read_lines(filename)) {
    const auto [gate_str, wire_name] = aoc::split_once(line, arrow_str);
    const auto output_wire = get_signal(wire_name);
    gate_t gate{};
    if (gate_str.starts_with(not_str)) {
      gate.op = operation_t::not_op;
      gate.rhs = get_signal(gate_str.substr(not_str.size()));
    } else if (!gate_str.contains(' ')) {
      if (std::isdigit(gate_str[0])) {
        gate.op = operation_t::value_op;
      } else {
        gate.op = operation_t::alias_op;
      }
      gate.lhs = get_signal(gate_str);
    } else {
      const auto [lhs, op, rhs] = aoc::split_to_array<3>(gate_str, ' ');
      gate.lhs = get_signal(lhs);
      gate.op = [&]() {
        if (op == and_str) {
          return operation_t::and_op;
        }
        if (op == or_str) {
          return operation_t::or_op;
        }
        if (op == lshift_str) {
          return operation_t::lshift_op;
        }
        if (op == rshift_str) {
          return operation_t::rshift_op;
        }
        AOC_UNREACHABLE("Invalid operation!");
      }();
      gate.rhs = get_signal(rhs);
    }
    const auto index = input_id(output_wire);
    inputs.resize(std::max(static_cast<size_t>(index + 1), inputs.size()));
    inputs[index] = gate;
  }
  return {inputs, get_signal("a"), get_signal("b")};
}

constexpr strict_signal_t solve_wire(const std::span<gate_t> inputs,
                                     const signal_t start) {
  if (is_value(start)) {
    return strict(start);
  }
  auto& gate = inputs[input_id(start)];
  // Note that all operations would undergo integral promotion anyway,
  // so we just cast the result instead of the operands
  switch (gate.op) {
    using enum operation_t;
    case value_op:
      return strict(gate.lhs);
    case alias_op:
      gate.lhs = solve_wire(inputs, gate.lhs);
      return strict(gate.lhs);
    case not_op:
      gate.rhs = solve_wire(inputs, gate.rhs);
      // We need to explicitly mask the lower 16 bits to avoid UB
      return strict(~gate.rhs & (first_wire_id - 1));
    default:
      gate.lhs = solve_wire(inputs, gate.lhs);
      gate.rhs = solve_wire(inputs, gate.rhs);
      break;
  }
  switch (gate.op) {
    using enum operation_t;
    case and_op:
      return strict(gate.lhs & gate.rhs);
    case or_op:
      return strict(gate.lhs | gate.rhs);
    case lshift_op:
      return strict(gate.lhs << gate.rhs);
    case rshift_op:
      return strict(gate.lhs >> gate.rhs);
    default:
      AOC_UNREACHABLE("Operation should have been covered before!");
  }
}

template <bool override>
strict_signal_t solve_case(const std::string& filename) {
  auto [inputs, a, b] = parse(filename);
  strict_signal_t result = 0;
  {
    auto inputs_copy = [&]() {
      if constexpr (override) {
        return inputs;
      } else {
        return std::move(inputs);
      }
    }();
    result = solve_wire(inputs_copy, a);
  }
  if constexpr (override) {
    inputs[input_id(b)] = gate_t{
        .op = operation_t::value_op,
        .lhs = result,
        .rhs = 0,
    };
    result = solve_wire(inputs, a);
  }
  return result;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(114, solve_case<false>("day07.example"));
  AOC_EXPECT_RESULT(16076, solve_case<false>("day07.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(28, solve_case<true>("day07.example"));
  AOC_EXPECT_RESULT(2797, solve_case<true>("day07.input"));
  AOC_RETURN_CHECK_RESULT();
}
