// https://adventofcode.com/2022/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <future>
#include <limits>
#include <print>
#include <ranges>
#include <unordered_map>

constexpr let NUM_ROBOTS = 4uz;
constexpr let NUM_RESOURCES = NUM_ROBOTS;
using Robots = std::array<u8, NUM_ROBOTS>;
using Resources = std::array<u8, NUM_RESOURCES>;
using Blueprint = std::array<Resources, NUM_ROBOTS>;
using Time = u8;

namespace stdv = std::views;

constexpr let GEODE_ID = NUM_ROBOTS - 1;

fn parse(String const& filename) -> Vec<Blueprint> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let[_, costs] = aoc::split_once(line, ':');
           let[ore_robot, clay_robot, obsidian_robot, geode_robot] =
               aoc::split_to_array<4>(costs, '.');

           let ore_cost = aoc::split_to_array<1>(
               ore_robot.substr(sizeof("Each ore robot costs ")), ' ');
           let clay_cost = aoc::split_to_array<1>(
               clay_robot.substr(sizeof("Each clay robot costs ")), ' ');
           let obs_cost = aoc::split_to_array<4>(
               obsidian_robot.substr(sizeof("Each obsidian robot costs ")),
               ' ');
           let geo_cost = aoc::split_to_array<4>(
               geode_robot.substr(sizeof("Each geode robot costs ")), ' ');

           let n = [](str s) { return static_cast<u8>(aoc::to_number<u8>(s)); };
           return Blueprint{
               Resources{n(ore_cost[0]), 0, 0, 0},
               Resources{n(clay_cost[0]), 0, 0, 0},
               Resources{n(obs_cost[0]), n(obs_cost[3]), 0, 0},
               Resources{n(geo_cost[0]), 0, n(geo_cost[3]), 0},
           };
         }) |
         aoc::ranges::to<Vec<Blueprint>>();
}

/// robots and time_left are packed into a u32:
/// - robots: u6[4] (bits 0-23)
/// - time_left: u8 (bits 24-31)
struct SearchNode {
  u32 packed{};
  Resources resources{};

  static constexpr let ROBOTS_MASK = u32{0x3f};
  static constexpr let ROBOTS_SHIFT = std::array<u32, NUM_ROBOTS>{0, 6, 12, 18};
  static constexpr let TIME_SHIFT = u32{24};

  constexpr SearchNode() = default;
  constexpr SearchNode(Robots const& robots, Resources const& resources,
                       Time time_left)
      : resources{resources} {
    for (usize i = 0; i < NUM_ROBOTS; ++i) {
      set_robot(i, robots[i]);
    }
    set_time_left(time_left);
  }

  fn robots(usize idx) const -> u8 {
    return static_cast<u8>((packed >> ROBOTS_SHIFT[idx]) & ROBOTS_MASK);
  }

  constexpr void set_robot(usize idx, u8 value) {
    let shift = ROBOTS_SHIFT[idx];
    let mask = ROBOTS_MASK << shift;
    packed &= ~mask;
    packed |= static_cast<u32>(value) << shift;
  }

  fn time_left() const -> Time {
    return static_cast<Time>(packed >> TIME_SHIFT);
  }

  constexpr void set_time_left(Time value) {
    let mask = u32{0xff} << TIME_SHIFT;
    packed &= ~mask;
    packed |= static_cast<u32>(value) << TIME_SHIFT;
  }

  fn robots_array() const -> Resources {
    return {robots(0), robots(1), robots(2), robots(3)};
  }

  fn collect_resources() const -> Resources {
    auto new_resources = resources;
    for (usize robot_id = 0; robot_id < NUM_ROBOTS; ++robot_id) {
      let sum = static_cast<u16>(new_resources[robot_id] + robots(robot_id));
      new_resources[robot_id] = static_cast<u8>(
          std::min(sum, static_cast<u16>(std::numeric_limits<u8>::max())));
    }
    return new_resources;
  }

  constexpr bool operator==(SearchNode const&) const = default;
};

template <>
struct std::hash<SearchNode> {
  fn operator()(SearchNode const& search_node) const noexcept -> usize {
    auto h = std::hash<u32>{}(search_node.packed);
    for (u8 r : search_node.resources) {
      h = h * 131 + r;
    }
    return h;
  }
};

fn clamp_resources(Resources const& max_resources, Resources const& resources,
                   Time time_left) -> Resources {
  auto result = resources;
  for (usize id = 0; id < GEODE_ID; ++id) {
    let max_spend = static_cast<u16>(max_resources[id] * time_left);
    result[id] =
        static_cast<u8>(std::min(static_cast<u16>(result[id]), max_spend));
  }
  return result;
}

fn try_build_robot(Blueprint const& blueprint, Resources const& max_resources,
                   SearchNode const& search_node, usize robot_id)
    -> Option<std::pair<Time, Resources>> {
  for (usize cost_id = 0; cost_id < NUM_ROBOTS; ++cost_id) {
    if (search_node.robots(cost_id) > max_resources[cost_id]) {
      return None;
    }
  }

  auto new_node = search_node;
  for (Time minute = 0; minute < search_node.time_left(); ++minute) {
    let time_left = new_node.time_left();

    auto new_resources = new_node.resources;
    bool could_build = true;
    for (usize cost_id = 0; cost_id < NUM_ROBOTS; ++cost_id) {
      let cost = blueprint[robot_id][cost_id];
      if (new_node.resources[cost_id] < cost) {
        could_build = false;
        break;
      }
      new_resources[cost_id] -= cost;
    }

    if (could_build) {
      new_node.resources = new_resources;
    }

    new_node.resources = new_node.collect_resources();

    if (could_build) {
      return std::pair{static_cast<Time>(minute + 1), new_node.resources};
    }
    new_node.set_time_left(static_cast<Time>(time_left - 1));
  }

  return None;
}

using Cache = std::unordered_map<SearchNode, u16>;

fn max_open_geodes_cached(Cache& cache, u16& current_best,
                          Blueprint const& blueprint,
                          Resources const& max_resources,
                          SearchNode search_node) -> u16 {
  let time_left = search_node.time_left();
  let current_geodes = static_cast<u16>(search_node.resources[GEODE_ID]);
  if (time_left == 0) {
    return current_geodes;
  }

  let robots = search_node.robots_array();
  let resources = search_node.resources;

  let cache_node = SearchNode{
      robots, clamp_resources(max_resources, resources, time_left), time_left};
  if (let it = cache.find(cache_node); it != cache.end()) {
    return it->second;
  }

  let default_until_end = static_cast<u16>(
      current_geodes + search_node.robots(GEODE_ID) * time_left);
  current_best = std::max(current_best, default_until_end);

  let time = static_cast<u16>(time_left);
  let max_potential_geodes = current_geodes +
                             static_cast<u16>(robots[GEODE_ID]) * time +
                             (time * (time - 1)) / 2;

  auto num_geodes = default_until_end;
  for (usize robot_id = NUM_ROBOTS; robot_id-- > 0;) {
    if (current_best > max_potential_geodes) {
      break;
    }
    if (let result =
            try_build_robot(blueprint, max_resources, search_node, robot_id)) {
      let[time_to_build, new_resources] = *result;
      auto new_node = SearchNode{robots, new_resources,
                                 static_cast<Time>(time_left - time_to_build)};
      new_node.set_robot(robot_id, new_node.robots(robot_id) + 1);
      let variant_geodes = max_open_geodes_cached(
          cache, current_best, blueprint, max_resources, new_node);
      current_best = std::max(current_best, variant_geodes);
      num_geodes = std::max(num_geodes, variant_geodes);
    }
  }

  cache[cache_node] = num_geodes;
  return num_geodes;
}

fn compute_max_costs(Blueprint const& blueprint) -> Resources {
  auto result = Resources{};
  for (let& robot_costs : blueprint) {
    for (usize i = 0; i < NUM_ROBOTS; ++i) {
      result[i] = std::max(result[i], robot_costs[i]);
    }
  }
  result[GEODE_ID] = std::numeric_limits<u8>::max();
  return result;
}

fn max_open_geodes(Blueprint const& blueprint, Time time_left) -> u16 {
  auto cache = Cache{};
  auto current_best = u16{};
  return max_open_geodes_cached(
      cache, current_best, blueprint, compute_max_costs(blueprint),
      SearchNode{{1, 0, 0, 0}, Resources{}, time_left});
}

fn solve_case1(Vec<Blueprint> const& blueprints) -> u16 {
  return aoc::ranges::accumulate(
      blueprints | stdv::enumerate | stdv::transform([](let blue_pair) {
        let & [ id, blueprint ] = blue_pair;
        return static_cast<u16>(id + 1) * max_open_geodes(blueprint, 24);
      }),
      u16{});
}

fn solve_case2(Vec<Blueprint> const& blueprints) -> u16 {
  auto futures = Vec<std::future<u16>>{};
  for (let& blueprint : blueprints | stdv::take(3)) {
    futures.push_back(std::async(std::launch::async, [&blueprint] {
      return max_open_geodes(blueprint, 32);
    }));
  }
  return std::ranges::fold_left(
      futures | stdv::transform([](auto& f) { return f.get(); }), u16{1},
      std::multiplies{});
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(33, solve_case1(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(1395, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(56 * 62, solve_case2(example));
  AOC_EXPECT_RESULT(2700, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
