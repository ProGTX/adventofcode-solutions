// https://adventofcode.com/2022/day/19

#include "../common/common.h"
#include "../common/print.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

enum type_t {
  ore = 0,
  clay = 1,
  obsidian = 2,
  geode = 3,
};

struct resource_pack {
  using value_type = int;
  using container_type = std::array<value_type, 4>;

  container_type resources{0, 0, 0, 0};

  constexpr resource_pack& operator+=(const resource_pack& rhs) {
    for (int r = 0; r < resources.size(); ++r) {
      resources[r] += rhs.resources[r];
    }
    return *this;
  }
  constexpr friend resource_pack operator+(resource_pack lhs,
                                           const resource_pack& rhs) {
    lhs += rhs;
    return lhs;
  }

  constexpr resource_pack& operator-=(const resource_pack& rhs) {
    for (int r = 0; r < resources.size(); ++r) {
      resources[r] -= rhs.resources[r];
    }
    return *this;
  }
  constexpr friend resource_pack operator-(resource_pack lhs,
                                           const resource_pack& rhs) {
    lhs -= rhs;
    return lhs;
  }

  constexpr bool is_sufficient() const {
    return std::ranges::all_of(resources,
                               [](value_type value) { return value >= 0; });
  }

  constexpr value_type& operator[](int index) { return resources[index]; }
  constexpr const value_type& operator[](int index) const {
    return resources[index];
  }

  auto begin() const { return std::begin(resources); }
  auto end() const { return std::end(resources); }
};

using blueprint_t =
    std::array<resource_pack,
               std::tuple_size<resource_pack::container_type>::value>;

std::optional<resource_pack> try_consume_robot_resources(
    const resource_pack& resources, const resource_pack& new_robot_cost) {
  auto new_resources = resources - new_robot_cost;
  if (new_resources.is_sufficient()) {
    std::cout << "  Start building new robot by spending: "
              << print_range(new_robot_cost) << std::endl;
    return new_resources;
  } else {
    return std::nullopt;
  }
}

template <int minutes>
int num_opened_geodes(const blueprint_t& blueprint,
                      const resource_pack& max_costs, resource_pack robots,
                      resource_pack resources, int minute) {
  const auto build_robots = [&]() -> resource_pack {
    resource_pack new_robots;
    bool made_robot = false;
    const auto try_make_robot = [&](type_t robot_type) {
      if (made_robot) {
        // Can only make one robot per turn
        return;
      }
      if (robots[robot_type] >= max_costs[robot_type]) {
        // Don't build more robots that needed
        return;
      }
      const auto new_resources =
          try_consume_robot_resources(resources, blueprint[robot_type]);
      if (new_resources.has_value()) {
        // We have sufficient resources to make the robot
        made_robot = true;
        resources = new_resources.value();
        ++new_robots[robot_type];
      }
    };

    try_make_robot(geode);
    try_make_robot(obsidian);
    try_make_robot(clay);
    try_make_robot(ore);

    return new_robots;
  };

  for (; minute < minutes; ++minute) {
    std::cout << "Minute " << (minute + 1) << std::endl;
    auto new_robots = build_robots();
    resources += robots;
    robots += new_robots;
  }

  std::cout << "Total resources: " << print_range(resources) << std::endl
            << std::endl;

  return resources[geode];
}

template <int minutes>
int solve_case(const std::string& filename) {
  std::vector<resource_pack> max_costs;
  std::vector<blueprint_t> blueprints;

  readfile_op(filename, [&](std::string_view line) {
    auto [blueprint_id, costs] = split<std::array<std::string, 2>>(line, ':');
    auto [ore_robot, clay_robot, obsidian_robot, geode_robot] =
        split<std::array<std::string, 4>>(costs, '.');

    auto ore_robot_costs = split<std::array<std::string, 1>>(
        ore_robot.substr(sizeof("Each ore robot costs")), ' ');
    auto clay_robot_costs = split<std::array<std::string, 1>>(
        clay_robot.substr(sizeof("Each clay robot costs")), ' ');
    auto obsidian_robot_costs = split<std::array<std::string, 4>>(
        obsidian_robot.substr(sizeof("Each obsidian robot costs")), ' ');
    auto geode_robot_costs = split<std::array<std::string, 4>>(
        geode_robot.substr(sizeof("Each geode robot costs")), ' ');

    blueprints.push_back({
        resource_pack{to_number<int>(ore_robot_costs[0]), 0, 0, 0},
        resource_pack{to_number<int>(clay_robot_costs[0]), 0, 0, 0},
        resource_pack{to_number<int>(obsidian_robot_costs[0]),
                      to_number<int>(obsidian_robot_costs[3]), 0, 0},
        resource_pack{to_number<int>(geode_robot_costs[0]), 0,
                      to_number<int>(geode_robot_costs[3]), 0},
    });
    const blueprint_t& bp = blueprints.back();

    max_costs.push_back({
        std::ranges::max({bp[0][0], bp[1][0], bp[2][0], bp[3][0]}),
        std::ranges::max({bp[0][1], bp[1][1], bp[2][1], bp[3][1]}),
        std::ranges::max({bp[0][2], bp[1][2], bp[2][2], bp[3][2]}),
        2'000'000'000, // It's always worth building geode cracking robots
    });
  });

  int quality_level = 0;
  for (int index = 0; index < blueprints.size(); ++index) {
    quality_level +=
        (index + 1) * num_opened_geodes<minutes>(
                          blueprints[index], max_costs[index],
                          resource_pack{1, 0, 0, 0}, resource_pack{}, 0);
  }

  std::cout << filename << " -> " << quality_level << std::endl;
  return quality_level;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(33, solve_case<24>("day19.example"));
  // AOC_EXPECT_RESULT(33, solve_case<24>("day19.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(10, solve_case<24>("day19.example"));
  // solve_case<24>("day19.input");
  AOC_RETURN_CHECK_RESULT();
}
