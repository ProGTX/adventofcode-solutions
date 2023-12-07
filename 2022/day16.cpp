// https://adventofcode.com/2022/day/16

#include "../common/common.h"
#include "../common/grid.h"
#include "../common/print.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

template <class Name>
struct valve_t {
  using name_t = Name;
  using tunnels_t = std::vector<name_t>;

  name_t name;
  int rate;
  tunnels_t tunnels;

  friend bool operator==(const valve_t& lhs, const valve_t& rhs) {
    return lhs.name == rhs.name;
  }
  friend bool operator==(const valve_t& lhs, const name_t& name) {
    return lhs.name == name;
  }
};

using named_valves_t = std::vector<valve_t<std::string>>;
using valves_t = std::vector<valve_t<int>>;
// Using string instead of vector gives us SSO
using path_t = std::string;

bool on_path(const auto& path, const auto& valve) {
  auto index = 0;
  if constexpr (std::same_as<std::decay_t<decltype(valve)>, valve_t<int>>) {
    index = valve.name;
  } else {
    index = valve;
  }
  return std::ranges::find(path, index) != std::end(path);
};

int release_pressure(const valves_t& valves, path_t path,
                     const int current_index, const int current_release,
                     const int cumulative_release, const int num_minutes) {
  if (num_minutes <= 0) {
    return cumulative_release + current_release;
  }

  const auto& current = valves[current_index];
  std::vector<int> release_values;
  release_values.reserve(current.tunnels.size() + 1);

  if ((current.rate > 0) && !on_path(path, current_index)) {
    path.push_back(current_index);
    release_values.push_back(release_pressure(
        valves, path, current_index, current_release + current.rate,
        cumulative_release + current_release, num_minutes - 1));
    path.resize(path.size() - 1);
  }

  for (const auto& neighbor : current.tunnels) {
    path.push_back(current_index);
    release_values.push_back(release_pressure(
        valves, path, neighbor, current_release,
        cumulative_release + current_release, num_minutes - 1));
    path.resize(path.size() - 1);
  }

  return std::ranges::max(release_values);
}

template <int num_minutes, bool>
int solve_case(const std::string& filename) {
  named_valves_t named_valves;

  readfile_op(filename, [&](std::string_view line) {
    auto [valve_info, tunnel_info] =
        split<std::array<std::string, 2>>(line, ';');

    auto [vstr0, name, vstr1, vstr2, rate_str] =
        split<std::array<std::string, 5>>(valve_info, ' ');

    auto rate = to_number<int>(rate_str.substr(sizeof("rate")));
    tunnel_info = tunnel_info.substr(sizeof("tunnels lead to valves"));
    auto tunnels =
        split<std::vector<std::string>, decltype(get_trimmer<std::string>())>(
            tunnel_info, ',');

    named_valves.emplace_back(std::move(name), rate, std::move(tunnels));
  });

  const auto named_valves_begin = std::begin(named_valves);
  const auto name_to_index = [&](std::string_view name) {
    auto it = std::find_if(
        std::begin(named_valves), std::end(named_valves),
        [&](const valve_t<std::string>& valve) { return valve.name == name; });
    return static_cast<int>(std::distance(named_valves_begin, it));
  };

  valves_t valves;
  std::ranges::transform(named_valves, std::back_inserter(valves),
                         [&](const valve_t<std::string>& valve) {
                           std::vector<int> tunnels;
                           tunnels.reserve(valve.tunnels.size());
                           std::ranges::transform(valve.tunnels,
                                                  std::back_inserter(tunnels),
                                                  name_to_index);
                           return valve_t<int>{name_to_index(valve.name),
                                               valve.rate, std::move(tunnels)};
                         });
  std::cout << print_range(
                   valves |
                       std::views::transform([&](const valve_t<int>& valve) {
                         return printable_tuple{valve.name,
                                                named_valves[valve.name].name,
                                                valve.rate, valve.tunnels};
                       }),
                   "\n")
            << std::endl;

  auto score =
      release_pressure(valves, {}, name_to_index("AA"), 0, 0, num_minutes);

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<25, false>("day16.example");
  // solve_case<30, false>("day16.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<30, true>("day16.example");
  // solve_case<30, true>("day16.input");
  AOC_RETURN_CHECK_RESULT();
}
