// https://adventofcode.com/2022/day/16

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <limits>
#include <print>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace stdv = std::views;

struct Valve {
  u32 flow_rate;
  Vec<usize> tunnels;
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
    let flow_rate = aoc::to_number<u32>(aoc::split_once(rate_part, '=')[1]);

    // "... tunnels lead to valves DD, II, BB"  or
    // "... tunnel leads to valve GG"
    auto neighbors_str = rest.substr(rest.find("valve") + sizeof("valve") - 1);
    if (!neighbors_str.empty() && neighbors_str.front() == 's') {
      neighbors_str = neighbors_str.substr(1);
    }
    let tunnels = aoc::split(neighbors_str, ',', aoc::trimmer{}) |
                  stdv::transform([&](str n) { return name_to_id.expect(n); }) |
                  aoc::ranges::to<Vec<usize>>();

    valves[id] = Valve{flow_rate, std::move(tunnels)};
  }

  return {std::move(valves), name_to_id.expect("AA")};
}

fn bfs_distances(Valves const& valves, usize start) -> Vec<u32> {
  constexpr let max_distance = std::numeric_limits<u32>::max();
  auto distances = Vec<u32>(valves.size(), max_distance);
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

struct SearchState {
  usize current_id;
  u32 time_left;
  Vec<bool> opened;
};

fn find_most_pressure(const Valves& valves, const Vec<Vec<u32>>& distances,
                      SearchState state, u32 total_flow, u32 total_pressure)
    -> u32 {
  auto most_pressure = total_pressure + total_flow * state.time_left;
  for (let& [ neighbor_id, neighbor ] : valves | stdv::enumerate) {
    // Find valves to open
    if (state.opened[neighbor_id] || neighbor.flow_rate == 0) {
      continue;
    }
    // +1 because it takes 1 minute to open the valve
    let time_to_open = distances[state.current_id][neighbor_id] + 1;
    if (time_to_open > state.time_left) {
      continue;
    }
    // Move to neighbor valve and open it
    auto next_opened = state.opened;
    next_opened[neighbor_id] = true;
    most_pressure =
        std::max(most_pressure,
                 find_most_pressure(
                     valves, distances,
                     SearchState{.current_id = static_cast<usize>(neighbor_id),
                                 .time_left = state.time_left - time_to_open,
                                 .opened = std::move(next_opened)},
                     total_flow + neighbor.flow_rate,
                     total_pressure + total_flow * time_to_open));
  }
  return most_pressure;
}

template <u32 num_minutes>
fn solve_case1(const Input& input) -> u32 {
  let num_valves = input.valves.size();
  let distances = Range{0uz, num_valves} |
                  stdv::transform([&](let neighbor_id) {
                    return bfs_distances(input.valves, neighbor_id);
                  }) |
                  aoc::ranges::to<Vec<Vec<u32>>>();
  return find_most_pressure( //
      input.valves, distances,
      SearchState{.current_id = input.aa_id,
                  .time_left = num_minutes,
                  .opened = Vec<bool>(num_valves, false)},
      0, 0);
}

int main() {
  std::println("Part 1");
  let example = parse("day16.example");
  AOC_EXPECT_RESULT(1651, solve_case1<30>(example));
  let input = parse("day16.input");
  AOC_EXPECT_RESULT(1647, solve_case1<30>(input));

  std::println("Part 2");
  AOC_RETURN_CHECK_RESULT();
}
