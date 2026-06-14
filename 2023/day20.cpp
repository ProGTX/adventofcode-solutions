// https://adventofcode.com/2023/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <map>
#include <print>
#include <variant>

using signal_t = bool;
using inputs_t = std::map<usize, signal_t>;
using outputs_t = aoc::static_vector<usize, 7>;
using output_names_t = aoc::static_vector<String, 7>;
constexpr str broadcaster_str = "broadcaster";
constexpr let button_id = static_cast<usize>(-1);

struct flip_flop_t {
  inputs_t inputs;
  outputs_t outputs;
  signal_t state = false;
};

struct conjunction_t {
  inputs_t inputs;
  outputs_t outputs;
};

struct broadcast_t {
  inputs_t inputs;
  outputs_t outputs;
};

struct output_t {
  outputs_t outputs;
};

using module_t =
    std::variant<output_t, flip_flop_t, conjunction_t, broadcast_t>;

fn module_outputs(module_t const& module) -> outputs_t const& {
  return std::visit([](auto&& m) -> outputs_t const& { return m.outputs; },
                    module);
}

fn module_add_input(module_t& module, usize name, signal_t input) {
  std::visit(
      [&](auto&& m) {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::same_as<T, output_t>) {
          // Output module ignores its inputs
        } else if constexpr (std::same_as<T, flip_flop_t>) {
          m.inputs.clear();
          m.inputs[name] = input;
        } else {
          m.inputs[name] = input;
        }
      },
      module);
}

fn module_process(module_t& module) -> Option<signal_t> {
  return std::visit(
      [](auto&& m) -> Option<signal_t> {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::same_as<T, flip_flop_t>) {
          AOC_ASSERT(m.inputs.size() == 1,
                     "Flip flop requires exactly one input");
          if (m.inputs.begin()->second) {
            // Ignore high pulse
            return None;
          }
          m.state = !m.state;
          return m.state;
        } else if constexpr (std::same_as<T, conjunction_t>) {
          return !stdr::all_of(
              m.inputs, [](auto&& input_pair) { return input_pair.second; });
        } else if constexpr (std::same_as<T, broadcast_t>) {
          AOC_ASSERT(m.inputs.size() == 1,
                     "Broadcast requires exactly one input");
          return m.inputs.begin()->second;
        } else {
          return None;
        }
      },
      module);
}

struct signal_transit_t {
  usize from;
  signal_t signal;
  usize to;
};

using module_map_t = Vec<module_t>;

struct push_result_t {
  std::array<i32, 2> num_pulses{0, 0};
  bool target_received_low = false;
};

fn push_button(module_map_t& module_map, usize broadcaster_id,
               Option<usize> target_id = None) -> push_result_t {
  push_result_t result;
  auto remaining_signals = Vec<signal_transit_t>{};

  // Push button
  remaining_signals.emplace_back(button_id, false, broadcaster_id);
  ++result.num_pulses[static_cast<usize>(false)];

  // Process signals
  for (auto signals_start = 0uz; signals_start < remaining_signals.size();
       ++signals_start) {
    let current_signal = remaining_signals[signals_start];
    auto& current = module_map.at(current_signal.to);
    module_add_input(current, current_signal.from, current_signal.signal);

    let signal = module_process(current);
    if (!signal.has_value()) {
      continue;
    }
    let& outputs = module_outputs(current);
    remaining_signals.reserve(remaining_signals.size() + outputs.size());
    for (let out : outputs) {
      remaining_signals.emplace_back(current_signal.to, *signal, out);
      ++result.num_pulses[static_cast<usize>(*signal)];
      if (target_id.has_value() && (out == *target_id) && !*signal) {
        result.target_received_low = true;
      }
    }
  }
  return result;
}

struct input_t {
  module_map_t module_map;
  usize broadcaster_id;
  Option<usize> rx_id;
};

fn parse(String const& filename) -> input_t {
  auto name_to_id = aoc::name_to_id{};
  let broadcaster_id = name_to_id.intern(broadcaster_str);

  auto module_map = module_map_t{};

  // Need to track the inputs
  // so that we can set the inputs correctly for the conjunctions
  auto input_map = std::map<usize, Vec<usize>>{};

  for (str line : aoc::views::read_lines(filename)) {
    let[from, to] = aoc::split_once<String>(line, " -> ");
    let name = ((from[0] == '%') || (from[0] == '&')) ? from.substr(1) : from;
    let id = name_to_id.intern(name);
    module_map.resize(name_to_id.new_size(module_map.size()));

    outputs_t outputs;
    for (let& out_name : aoc::split<output_names_t>(to, ", ")) {
      let out_id = name_to_id.intern(out_name);
      module_map.resize(name_to_id.new_size(module_map.size()));
      outputs.push_back(out_id);
      input_map[out_id].push_back(id);
    }

    if (from[0] == '%') {
      module_map[id] = flip_flop_t{.outputs = std::move(outputs)};
    } else if (from[0] == '&') {
      module_map[id] = conjunction_t{.outputs = std::move(outputs)};
    } else if (from == broadcaster_str) {
      module_map[id] = broadcast_t{.outputs = std::move(outputs)};
    }
  }

  // Only conjunctions with more than one input need to have inputs updated
  for (let& [ id, inputs ] : input_map) {
    if (inputs.size() < 2) {
      continue;
    }
    for (let input_id : inputs) {
      module_add_input(module_map[id], input_id, false);
    }
  }

  return input_t{std::move(module_map), broadcaster_id, name_to_id.get("rx")};
}

fn solve_case1(input_t input) -> u64 {
  auto total_signals = std::array<u64, 2>{0, 0};
  for (let _ : Range{0, 1000}) {
    let result = push_button(input.module_map, input.broadcaster_id);
    total_signals[0] += static_cast<u64>(result.num_pulses[0]);
    total_signals[1] += static_cast<u64>(result.num_pulses[1]);
  }
  return total_signals[0] * total_signals[1];
}

fn solve_case2(input_t input) -> u64 {
  let rx_id = input.rx_id.value();
  for (let num_presses : stdv::iota(1)) {
    let result = push_button(input.module_map, input.broadcaster_id, rx_id);
    if (result.target_received_low) {
      return static_cast<u64>(num_presses);
    }
  }
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(32000000, solve_case1(example));
  let example2 = parse("day20.example2");
  AOC_EXPECT_RESULT(11687500, solve_case1(example2));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(814934624, solve_case1(input));

  std::println("Part 2");
  aoc::return_incomplete();
  // AOC_EXPECT_RESULT(1337, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
