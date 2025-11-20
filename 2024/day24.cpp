// https://adventofcode.com/2024/day/24

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const int op_and = 0;
constexpr const int op_or = 1;
constexpr const int op_xor = 2;

constexpr const std::string_view wire_prefix = "x00";
constexpr const int z_wire_id_offset = 270;
constexpr const int uninit_wire = -1;

using int_t = std::int64_t;

struct gate_t {
  int lhs{uninit_wire};
  int rhs{uninit_wire};
  int op{uninit_wire};

  constexpr int get(std::span<const int> wires) const {
    AOC_ASSERT(op != uninit_wire, "Invalid gate");
    switch (op) {
      case op_and:
        return wires[lhs] & wires[rhs];
      case op_or:
        return wires[lhs] | wires[rhs];
      case op_xor:
        return wires[lhs] ^ wires[rhs];
      default:
        AOC_UNREACHABLE("Invalid op");
        return uninit_wire;
    }
  }
};

constexpr void solve_wire(std::span<int> wires, std::span<const gate_t> outputs,
                          int wire_id) {
  auto value = wires[wire_id];
  if (value != uninit_wire) {
    return;
  }
  const auto output_gate = outputs[wire_id];
  solve_wire(wires, outputs, output_gate.lhs);
  solve_wire(wires, outputs, output_gate.rhs);
  value = output_gate.get(wires);
  wires[wire_id] = value;
}

constexpr auto solve_wires(std::span<int> wires,
                           std::span<const gate_t> outputs,
                           const int input_id_end,
                           const int regular_wire_id_end) {
  for (int w = input_id_end; w < regular_wire_id_end; ++w) {
    solve_wire(wires, outputs, w);
  }
  for (int z = z_wire_id_offset; z < wires.size(); ++z) {
    solve_wire(wires, outputs, z);
  }
  return wires;
}

constexpr int_t get_z(std::span<const int> wires) {
  int_t number = 0;
  int_t multiplier = 1;
  for (const auto z_wire : wires.subspan(z_wire_id_offset)) {
    number += multiplier * z_wire;
    multiplier *= 2;
  }
  return number;
}

template <bool>
int_t solve_case(const std::string& filename) {
  aoc::flat_map<std::string, int> wire_ids;
  std::vector<int> wires;
  std::vector<gate_t> outputs;

  int wire_id = 0;
  const auto try_insert_wire = [&](const std::string& name) {
    auto it = wire_ids.find(name);
    if (it == wire_ids.end()) {
      auto id = wire_id;
      if (name[0] == 'z') {
        const auto z_wire_id = aoc::to_number<int>(name.substr(1));
        id = z_wire_id + z_wire_id_offset;
      } else {
        ++wire_id;
      }
      wires.resize(std::ranges::max(static_cast<int>(wires.size()), id + 1),
                   uninit_wire);
      wire_ids[name] = id;
    }
  };

  int input_id_end = 0;
  bool parsing_gates = false;
  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_gates = true;
      input_id_end = wire_id;
      continue;
    }
    if (!parsing_gates) {
      wire_ids[std::string{line.substr(0, wire_prefix.size())}] = wire_id;
      wires.push_back(aoc::to_number<int>(line.substr(wire_prefix.size() + 2)));
      ++wire_id;
    } else {
      const auto [input_str, output_str] = aoc::split_once(line, '-');
      const auto output_name = std::string{output_str.substr(2)};
      try_insert_wire(output_name);

      const auto [lhs_name, op_str, rhs_name, empty] =
          aoc::split<std::array<std::string, 4>>(input_str, ' ');
      try_insert_wire(lhs_name);
      try_insert_wire(rhs_name);

      const auto op_id = [&]() {
        if (op_str == "AND") {
          return op_and;
        } else if (op_str == "OR") {
          return op_or;
        } else if (op_str == "XOR") {
          return op_xor;
        }
        AOC_UNREACHABLE("Invalid op");
        return uninit_wire;
      }();

      outputs.resize(std::ranges::max(outputs.size(), wires.size()));
      outputs[wire_ids[output_name]] =
          gate_t{wire_ids[lhs_name], wire_ids[rhs_name], op_id};
    }
  }

  const auto regular_wire_id_end = wire_id;

  int_t sum = 0;
  sum = get_z(solve_wires(wires, outputs, input_id_end, regular_wire_id_end));
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(4, solve_case<false>("day24.example"));
  AOC_EXPECT_RESULT(2024, solve_case<false>("day24.example2"));
  AOC_EXPECT_RESULT(64755511006320, solve_case<false>("day24.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day24.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day24.input"));
  AOC_RETURN_CHECK_RESULT();
}
