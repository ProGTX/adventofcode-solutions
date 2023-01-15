// https://adventofcode.com/2021/day/7

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

using crabs_t = std::map<int, int>;

template <bool progressive_cost>
constexpr int_t get_move_cost(int_t cost) {
  if constexpr (progressive_cost) {
    return cost * (cost + 1) / 2;
  } else {
    return cost;
  }
}

template <bool progressive_cost>
int_t calc_cost(crabs_t const& crabs, const int position) {
  int_t cost = 0;
  for (auto&& [pos, num_crabs] : crabs) {
    int_t move_cost = get_move_cost<progressive_cost>(std::abs(position - pos));
    cost += move_cost * num_crabs;
  }
  return cost;
}

int_t lowest_fuel_deltas(crabs_t const& crabs) {
  static constexpr bool progressive_cost = false;
  AOC_ASSERT(std::ranges::is_sorted(crabs), "Crab positions must be sorted!");

  // We don't need to calculate the cost for each position,
  // we can just apply deltas from the first one
  int best_pos = *std::ranges::begin(crabs | std::views::keys);
  int_t lowest_cost = calc_cost<progressive_cost>(crabs, best_pos);
  const int_t num_all_crabs = std::invoke([&] {
    auto crab_values = crabs | std::views::values;
    return std::accumulate(crab_values.begin(), crab_values.end(), int_t{0});
  });

  int crabs_left = crabs.at(best_pos);
  int crabs_right = num_all_crabs - crabs_left;

  const auto last_pos =
      *std::ranges::begin(crabs | std::views::keys | std::views::reverse);
  for (int pos = best_pos + 1; pos < last_pos + 1; ++pos) {
    auto crab_it = crabs.find(pos);
    const int num_crabs = (crab_it == crabs.end()) ? 0 : crab_it->second;

    const int pos_diff = pos - best_pos;
    const int_t move_cost = get_move_cost<progressive_cost>(pos_diff);
    int_t new_cost = lowest_cost + move_cost * (crabs_left - crabs_right);

    if (new_cost < lowest_cost) {
      lowest_cost = new_cost;
      best_pos = pos;
    }

    crabs_left += num_crabs;
    crabs_right -= num_crabs;
  }

  return lowest_cost;
}

int_t lowest_fuel_all(crabs_t const& crabs) {
  static constexpr bool progressive_cost = true;
  AOC_ASSERT(std::ranges::is_sorted(crabs), "Crab positions must be sorted!");

  int best_pos = *std::ranges::begin(crabs | std::views::keys);
  int_t lowest_cost = calc_cost<progressive_cost>(crabs, best_pos);

  const auto last_pos =
      *std::ranges::begin(crabs | std::views::keys | std::views::reverse);
  for (int pos = best_pos + 1; pos < last_pos + 1; ++pos) {
    int_t new_cost = calc_cost<progressive_cost>(crabs, pos);

    if (new_cost < lowest_cost) {
      lowest_cost = new_cost;
      best_pos = pos;
    }
  }

  return lowest_cost;
}

template <bool progressive_cost>
int_t solve_case(const std::string& filename) {
  crabs_t crabs;

  readfile_op(filename, [&](std::string_view line) {
    auto crabs_pos = split<std::vector<int>>(line, ',');
    for (const int pos : crabs_pos) {
      auto crab_it = crabs.find(pos);
      if (crab_it == std::end(crabs)) {
        crabs[pos] = 1;
      } else {
        ++(crab_it->second);
      }
    }
  });

  int_t cost = 0;
  if constexpr (!progressive_cost) {
    cost = lowest_fuel_deltas(crabs);
  } else {
    cost = lowest_fuel_all(crabs);
  }
  std::cout << filename << " -> " << cost << std::endl;
  return cost;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(37, (solve_case<false>("day07.example")));
  AOC_EXPECT_RESULT(355592, (solve_case<false>("day07.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(168, (solve_case<true>("day07.example")));
  AOC_EXPECT_RESULT(101618069, (solve_case<true>("day07.input")));
  AOC_RETURN_CHECK_RESULT();
}
