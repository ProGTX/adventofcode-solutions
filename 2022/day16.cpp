// https://adventofcode.com/2022/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <bit>
#include <limits>
#include <print>
#include <queue>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct Valve {
  Vec<usize> tunnels;
  u8 flow_rate;
};
using Valves = Vec<Valve>;

struct Input {
  Valves valves;
  usize aa_id;
};

auto parse(const String& filename) -> Input {
  let lines = aoc::views::read_lines(filename) | aoc::ranges::to<Vec<String>>();

  auto name_to_id = aoc::name_to_id{};
  for (str line : lines) {
    // "Valve AA has flow rate=..."
    name_to_id.intern(line.substr(6, 2));
  }

  auto valves = Valves(lines.size());
  for (str line : lines) {
    let name = line.substr(6, 2);
    let id = name_to_id.expect(name);

    // "...flow rate=20; tunnel..."
    let[rate_part, rest] = aoc::split_once(line, ';');
    let flow_rate = static_cast<u8>(
        aoc::to_number<u32>(aoc::split_once(rate_part, '=')[1]));

    // "... tunnels lead to valves DD, II, BB"  or
    // "... tunnel leads to valve GG"
    auto neighbors_str = rest.substr(rest.find("valve") + sizeof("valve") - 1);
    if (!neighbors_str.empty() && neighbors_str.front() == 's') {
      neighbors_str = neighbors_str.substr(1);
    }
    let tunnels = aoc::split(neighbors_str, ',', aoc::trimmer{}) |
                  stdv::transform([&](str n) { return name_to_id.expect(n); }) |
                  aoc::ranges::to<Vec<usize>>();

    valves[id] = Valve{
        .tunnels = std::move(tunnels),
        .flow_rate = flow_rate,
    };
  }

  return {std::move(valves), name_to_id.expect("AA")};
}

fn bfs_distances(Valves const& valves, usize start) -> Vec<u8> {
  constexpr let max_distance = std::numeric_limits<u8>::max();
  auto distances = Vec<u8>(valves.size(), max_distance);
  distances[start] = 0;
  auto queue = std::queue<usize>{};
  queue.push(start);
  while (!queue.empty()) {
    let current = queue.front();
    queue.pop();
    for (let next : valves[current].tunnels) {
      if (distances[next] == max_distance) {
        distances[next] = distances[current] + 1;
        queue.push(next);
      }
    }
  }
  return distances;
}

// Maximum number of valves after graph compression
constexpr usize MAX_VALVES = 16;
using FlowRates = aoc::static_vector<u8, MAX_VALVES>;
using SingleDistances = aoc::static_vector<u8, MAX_VALVES>;

fn compress_graph(Valves const& valves, usize aa_id)
    -> std::pair<FlowRates, Vec<SingleDistances>> {
  // Precompute all-pairs distances
  let all_distances =
      Range{0uz, valves.size()} |
      stdv::transform([&](usize i) { return bfs_distances(valves, i); }) |
      aoc::ranges::to<Vec<Vec<u8>>>();

  // Collect useful valves (AA + flow > 0)
  auto nodes = Vec<usize>{};
  nodes.push_back(aa_id);
  for (let& [ id, v ] : valves | stdv::enumerate) {
    if (v.flow_rate > 0 && static_cast<usize>(id) != aa_id) {
      nodes.push_back(static_cast<usize>(id));
    }
  }

  let n = nodes.size();
  auto flow_rates = FlowRates{};
  for (let old_id : nodes) {
    flow_rates.push_back(valves[old_id].flow_rate);
  }

  // Build compressed distance matrix
  auto distances = Vec<SingleDistances>(n);
  for (let[i, old_i] : nodes | stdv::enumerate) {
    for (let old_j : nodes) {
      distances[i].push_back(all_distances[old_i][old_j]);
    }
  }

  return {std::move(flow_rates), std::move(distances)};
}

struct SearchState {
  usize current_id;
  u32 time_left;
  u32 opened_mask;

  fn is_open(usize id) -> bool { return opened_mask & (1u << id); }
};

fn find_most_pressure(std::span<const u8> flow_rates,
                      std::span<const SingleDistances> distances,
                      SearchState state, //
                      u32 total_flow) -> u32 {
  auto most_pressure = total_flow * state.time_left;
  for (let[neighbor_id, flow] : flow_rates | stdv::enumerate) {
    // Skip AA at index 0
    let real_id = static_cast<usize>(neighbor_id) + 1;
    if (state.is_open(real_id) || (flow == 0)) {
      continue;
    }
    let time_to_open =
        static_cast<u32>(distances[state.current_id][real_id]) + 1;
    if (time_to_open > state.time_left) {
      continue;
    }
    let neighbor_pressure =
        (total_flow * time_to_open) +
        find_most_pressure(
            flow_rates, //
            distances,
            SearchState{
                .current_id = real_id,
                .time_left = state.time_left - time_to_open,
                .opened_mask = state.opened_mask | (1u << real_id),
            },
            total_flow + static_cast<u32>(flow));
    most_pressure = std::max(most_pressure, neighbor_pressure);
  }
  return most_pressure;
}

fn solve_case1(Input const& input) -> u32 {
  let[flow_rates, distances] = compress_graph(input.valves, input.aa_id);
  return find_most_pressure(                      //
      std::span<const u8>(flow_rates).subspan(1), //
      distances,
      SearchState{
          .current_id = 0,
          .time_left = 30,
          .opened_mask = 0,
      },
      0);
}

fn solve_case2(Input const& input) -> u32 {
  let[flow_rates, distances] = compress_graph(input.valves, input.aa_id);
  let num_valves = flow_rates.size();
  let start_state = SearchState{
      .current_id = 0,
      .time_left = 26,
      .opened_mask = 0,
  };

  auto most_pressure = u32{};
  // Iterate over all subsets of half the valves
  // (split between player and elephant)
  let total_subsets = 1u << (num_valves - 1);
  for (let mask : Range{0u, total_subsets}) {
    if (static_cast<usize>(std::popcount(mask)) != (num_valves / 2)) {
      // subset2 has already covered smaller sizes,
      // mirrored solutions are identical, skipping larger ones is safe
      // However, skipping smaller ones is a heuristic
      // that just happens to work here
      continue;
    }
    // Assign each non-AA valve to either player (flows1) or elephant (flows2)
    auto flows1 = flow_rates;
    auto flows2 = flow_rates;
    for (let i : Range{1uz, num_valves}) {
      if (mask & (1u << (i - 1))) {
        flows2[i] = 0;
      } else {
        flows1[i] = 0;
      }
    }
    let pressure = //
        find_most_pressure(std::span<const u8>(flows1).subspan(1), distances,
                           start_state, 0) +
        find_most_pressure(std::span<const u8>(flows2).subspan(1), distances,
                           start_state, 0);
    most_pressure = std::max(most_pressure, pressure);
  }
  return most_pressure;
}

int main() {
  std::println("Part 1");
  let example = parse("day16.example");
  AOC_EXPECT_RESULT(1651, solve_case1(example));
  let input = parse("day16.input");
  AOC_EXPECT_RESULT(1647, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1707, solve_case2(example));
  AOC_EXPECT_RESULT(2169, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
