// https://adventofcode.com/2023/day/20

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;
using int_t = std::uint64_t;
using signal_t = bool;
using inputs_t = std::map<std::string, signal_t>;
using outputs_t = aoc::static_vector<std::string, 7>;
constexpr inline auto broadcaster_str = "broadcaster";

struct module_t {
 protected:
  std::string m_name; // For debugging
  inputs_t m_inputs;
  outputs_t m_outputs;
  signal_t m_state = false;

 public:
  module_t(std::string name, outputs_t outputs)
      : m_name(std::move(name)), m_outputs{std::move(outputs)} {}

  virtual void add_input(const std::string& name, signal_t input) {
    m_inputs[name] = input;
  }

  const outputs_t& outputs() const { return m_outputs; }

  virtual std::optional<signal_t> process() = 0;

  virtual ~module_t() = default;
};

struct flip_flop_t final : public module_t {
  using module_t::module_t;

  void add_input(const std::string& name, signal_t input) override {
    m_inputs.clear();
    m_inputs[name] = input;
  }

  virtual std::optional<signal_t> process() override {
    AOC_ASSERT(m_inputs.size() == 1, "Flip flop requires exactly one input");
    if (m_inputs.begin()->second) {
      // Ignore high pulse
      return std::nullopt;
    }
    m_state = !m_state;
    return m_state;
  }
};

struct conjunction_t final : public module_t {
  using module_t::module_t;

  virtual std::optional<signal_t> process() override {
    return !std::ranges::all_of(
        m_inputs, [](auto&& input_pair) { return input_pair.second; });
  }
};

struct broadcast_t final : public module_t {
  using module_t::module_t;

  virtual std::optional<signal_t> process() override {
    AOC_ASSERT(m_inputs.size() == 1, "Broadcast requires exactly one input");
    return m_inputs.begin()->second;
  }
};

struct output_t final : public module_t {
  output_t() : module_t{"output", {}} {}

  virtual std::optional<signal_t> process() override { return std::nullopt; }
};

struct signal_transit_t {
  std::string from;
  signal_t signal;
  std::string to;

  friend std::ostream& operator<<(std::ostream& out,
                                  const signal_transit_t& current_signal) {
    out << current_signal.from << " -"
        << (current_signal.signal ? "high" : "low") << "-> "
        << current_signal.to;
    return out;
  }
};

using module_map_t = aoc::flat_map<std::string, std::unique_ptr<module_t>>;

std::array<int, 2> push_button(module_map_t& module_map) {
  // aoc::println("push_button");
  std::array<int, 2> num_pulses{0, 0};
  std::vector<signal_transit_t> remaining_signals;

  // Push button
  remaining_signals.emplace_back("button", false, broadcaster_str);
  ++num_pulses[static_cast<int>(false)];

  // Process signals
  for (size_t signals_start = 0; signals_start < remaining_signals.size();
       ++signals_start) {
    const auto current_signal = remaining_signals[signals_start];
    // std::cout << current_signal << std::endl;
    module_map.at(current_signal.to)
        ->add_input(current_signal.from, current_signal.signal);

    auto& current = module_map.at(current_signal.to);
    auto signal = current->process();
    if (!signal.has_value()) {
      continue;
    }
    const auto& outputs = current->outputs();
    remaining_signals.reserve(remaining_signals.size() + outputs.size());
    for (const std::string& out : outputs) {
      remaining_signals.emplace_back(current_signal.to, *signal, out);
      ++num_pulses[static_cast<int>(*signal)];
    }
  }
  // aoc::println("num pulses", aoc::print_range(num_pulses));
  return num_pulses;
}

template <bool>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  module_map_t module_map;

  // Need to track the inputs
  // so that we can set the inputs correctly for the conjunctions
  aoc::flat_map<std::string, std::vector<std::string>> input_map;

  // Need to track outputs to find out which ones should be output_t
  std::vector<std::string> output_list;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [from, to] = aoc::split_once<std::string>(line, " -> "sv);
    auto name = ((from[0] == '%') || (from[0] == '&')) ? from.substr(1) : from;
    auto outputs = aoc::split<outputs_t>(to, ", "sv);
    std::ranges::copy(outputs, std::back_inserter(output_list));
    for (const auto& out : outputs) {
      input_map[out].push_back(name);
    }
    if (from[0] == '%') {
      module_map.emplace(
          name, std::make_unique<flip_flop_t>(name, std::move(outputs)));
    } else if (from[0] == '&') {
      module_map.emplace(
          name, std::make_unique<conjunction_t>(name, std::move(outputs)));
    } else if (from == broadcaster_str) {
      module_map.emplace(
          name, std::make_unique<broadcast_t>(name, std::move(outputs)));
    }
  }

  // Only conjunctions with more than one input need to have inputs updated
  for (const auto& [name, inputs] : input_map) {
    if (inputs.size() < 2) {
      continue;
    }
    for (const auto& input : inputs) {
      module_map.at(name)->add_input(input, false);
    }
  }
  for (auto& out : output_list) {
    if (!module_map.contains(out)) {
      module_map.emplace(std::move(out), std::make_unique<output_t>());
    }
  }

  std::array<int_t, 2> total_signals{0, 0};
  for (int i = 0; i < 1000; ++i) {
    auto [num_low, num_high] = push_button(module_map);
    total_signals[0] += num_low;
    total_signals[1] += num_high;
  }
  aoc::println("total signals", aoc::print_range(total_signals));
  auto sum = total_signals[0] * total_signals[1];
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(32000000, (solve_case<false>("day20.example")));
  AOC_EXPECT_RESULT(11687500, (solve_case<false>("day20.example2")));
  AOC_EXPECT_RESULT(814934624, (solve_case<false>("day20.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(952408144115, (solve_case<true>("day20.example")));
  // AOC_EXPECT_RESULT(90111113594927, (solve_case<true>("day20.input")));
  AOC_RETURN_CHECK_RESULT();
}
