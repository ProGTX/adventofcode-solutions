// https://adventofcode.com/2022/day/16

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

#include "../common.h"

struct valve_t {
  using tunnels_t = std::vector<std::string>;

  std::string name;
  int rate;
  tunnels_t tunnels;
};

using valves_t = std::vector<valve_t>;

template <int num_minutes>
int release_pressure(valves_t& valves) {
  int total_released = 0;

  return total_released;
}

template <int num_minutes, bool>
void solve_case(const std::string& filename) {
  valves_t valves;

  readfile_op(filename, [&](std::string_view line) {
    auto [valve_info, tunnel_info] =
        split<std::array<std::string, 2>>(std::string{line}, ';');

    auto [vstr0, name, vstr1, vstr2, rate_str] =
        split<std::array<std::string, 5>>(valve_info, ' ');

    auto rate = std::stoi(std::string{rate_str.substr(sizeof("rate"))});
    tunnel_info = tunnel_info.substr(sizeof("tunnels lead to valves"));
    auto tunnels =
        split<std::vector<std::string>, decltype(get_trimmer<std::string>())>(
            tunnel_info, ',');

    valves.emplace_back(std::move(name), rate, std::move(tunnels));
  });

  auto score = release_pressure<num_minutes>(valves);

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<30, false>("day16.example");
  // solve_case<30, false>("day16.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<30, true>("day16.example");
  // solve_case<30, true>("day16.input");
}
