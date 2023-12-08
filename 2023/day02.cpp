// https://adventofcode.com/2023/day/2

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

struct cube_config_t {
  int red{0};
  int green{0};
  int blue{0};

  constexpr bool contains(const cube_config_t& other) const {
    return (red >= other.red) && (green >= other.green) && (blue >= other.blue);
  }
};

inline constexpr cube_config_t config1{12, 13, 14};

template <cube_config_t config, std::size_t Extent>
bool is_game_possible(std::span<const std::string_view, Extent> games) {
  for (auto game_str : games) {
    auto cubes_str = split<std::array<std::string_view, 3>>(game_str, ',');
    cube_config_t current_config;
    for (auto cube_config : cubes_str) {
      if (cube_config.empty()) {
        continue;
      }
      auto [number_str, color] =
          split<std::array<std::string_view, 2>>(trim(cube_config), ' ');
      auto number = to_number<int>(number_str);
      if (color == "red") {
        current_config.red = number;
      } else if (color == "green") {
        current_config.green = number;
      } else if (color == "blue") {
        current_config.blue = number;
      } else {
        AOC_ASSERT(false, "Invalid color");
      }
      if (!config.contains(current_config)) {
        return false;
      }
    }
  }
  return true;
}

template <cube_config_t config>
int solve_case(const std::string& filename) {
  int sum = 0;

  auto solver = [&](std::string_view line, int linenum) {
    const int id = linenum;
    auto [prefix, game_str] = split<std::array<std::string_view, 2>>(line, ':');
    auto games = split<std::vector<std::string_view>>(game_str, ';');
    bool possible =
        is_game_possible<config>(std::span<const std::string_view>{games});
    sum += static_cast<int>(possible) * id;
  };

  readfile_op(filename, solver);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Asserts" << std::endl;
  AOC_EXPECT_RESULT(
      true,
      (is_game_possible<config1>(std::span<const std::string_view>{
          {"3 blue, 4 red"sv, "1 red, 2 green, 6 blue"sv, "2 green"sv}})));
  AOC_EXPECT_RESULT(
      false, (is_game_possible<config1>(std::span<const std::string_view>{
                 {"8 green, 6 blue, 20 red"sv, "5 blue, 4 red, 13 green"sv,
                  "5 green, 1 red"sv}})));

  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(8, solve_case<config1>("day02.example"));
  AOC_EXPECT_RESULT(8, solve_case<config1>("day02.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day02.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
