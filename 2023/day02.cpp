// https://adventofcode.com/2023/day/2

#include "../common/common.h"

#include <algorithm>
#include <array>
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

  bool operator==(const cube_config_t&) const = default;

  constexpr bool contains(const cube_config_t& other) const {
    return (red >= other.red) && (green >= other.green) && (blue >= other.blue);
  }

  friend constexpr cube_config_t max(const cube_config_t& lhs,
                                     const cube_config_t& rhs) {
    return {std::max(lhs.red, rhs.red), std::max(lhs.green, rhs.green),
            std::max(lhs.blue, rhs.blue)};
  }

  constexpr int power() const { return red * green * blue; }
};

inline constexpr cube_config_t config1{12, 13, 14};
inline constexpr cube_config_t config2{0, 0, 0};

/// Returns 1 if game possible, 0 if not (part 1)
/// Returns > 1 if config == {0,0,0} (part 2)
template <cube_config_t config, std::size_t Extent>
int cube_power(std::span<const std::string_view, Extent> games) {
  static constexpr bool part2 = (config == config2);
  cube_config_t max_config{1, 1, 1};
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
      if constexpr (!part2) {
        if (!config.contains(current_config)) {
          return 0;
        }
      } else {
        max_config = max(max_config, current_config);
      }
    }
  }
  return max_config.power();
}

template <cube_config_t config>
int solve_case(const std::string& filename) {
  int sum = 0;

  auto solver = [&](std::string_view line, int linenum) {
    const int id = linenum;
    auto [prefix, game_str] = split<std::array<std::string_view, 2>>(line, ':');
    auto games = split<std::vector<std::string_view>>(game_str, ';');
    int power = cube_power<config>(std::span<const std::string_view>{games});
    if constexpr (config != config2) {
      power *= id;
    }
    sum += power;
  };

  readfile_op(filename, solver);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Asserts" << std::endl;
  AOC_EXPECT_RESULT(
      1, (cube_power<config1>(std::span<const std::string_view>{
             {"3 blue, 4 red"sv, "1 red, 2 green, 6 blue"sv, "2 green"sv}})));
  AOC_EXPECT_RESULT(
      0, (cube_power<config1>(std::span<const std::string_view>{
             {"8 green, 6 blue, 20 red"sv, "5 blue, 4 red, 13 green"sv,
              "5 green, 1 red"sv}})));
  AOC_EXPECT_RESULT(
      48, (cube_power<config2>(std::span<const std::string_view>{
              {"3 blue, 4 red"sv, "1 red, 2 green, 6 blue"sv, "2 green"sv}})));
  AOC_EXPECT_RESULT(
      1560, (cube_power<config2>(std::span<const std::string_view>{
                {"8 green, 6 blue, 20 red"sv, "5 blue, 4 red, 13 green"sv,
                 "5 green, 1 red"sv}})));

  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(8, solve_case<config1>("day02.example"));
  AOC_EXPECT_RESULT(2600, solve_case<config1>("day02.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(2286, solve_case<config2>("day02.example"));
  AOC_EXPECT_RESULT(86036, solve_case<config2>("day02.input"));
  AOC_RETURN_CHECK_RESULT();
}
