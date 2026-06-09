// https://adventofcode.com/2023/day/2

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>

using Game = Vec<String>;

auto parse(String const& filename) -> Vec<Game> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let[_, game_str] = aoc::split_once(line, ':');
           return aoc::split_to_vec<String>(game_str, ';');
         }) |
         aoc::ranges::to<Vec<Game>>();
}

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
template <cube_config_t config>
fn cube_power(std::span<const str> games) -> i32 {
  static constexpr bool part2 = (config == config2);
  cube_config_t max_config{1, 1, 1};
  for (let game_str : games) {
    auto cubes_str = aoc::split_to_array<3>(game_str, ',');
    cube_config_t current_config;
    for (let cube_str : cubes_str) {
      if (cube_str.empty()) {
        continue;
      }
      let[number_str, color] = aoc::split_once(aoc::trim(cube_str), ' ');
      let number = aoc::to_number<i32>(number_str);
      if (color == "red") {
        current_config.red = number;
      } else if (color == "green") {
        current_config.green = number;
      } else if (color == "blue") {
        current_config.blue = number;
      } else {
        AOC_UNREACHABLE("Invalid color");
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
fn solve_case(Vec<Game> const& games) -> i32 {
  auto sum = i32{};
  for (let[id, rounds] : games | stdv::enumerate) {
    let rounds_sv = rounds |
                    aoc::views::transform_cast<str>() |
                    aoc::ranges::to<Vec<str>>();
    auto power = cube_power<config>(std::span<const str>{rounds_sv});
    if constexpr (config != config2) {
      power *= id + 1;
    }
    sum += power;
  }
  return sum;
}

static_assert(1 ==
              cube_power<config1>(std::span<const str>{
                  {"3 blue, 4 red", "1 red, 2 green, 6 blue", "2 green"}}));
static_assert(0 == cube_power<config1>(std::span<const str>{
                       {"8 green, 6 blue, 20 red", "5 blue, 4 red, 13 green",
                        "5 green, 1 red"}}));
static_assert(48 ==
              cube_power<config2>(std::span<const str>{
                  {"3 blue, 4 red", "1 red, 2 green, 6 blue", "2 green"}}));
static_assert(1560 == cube_power<config2>(std::span<const str>{
                          {"8 green, 6 blue, 20 red", "5 blue, 4 red, 13 green",
                           "5 green, 1 red"}}));

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT(8, solve_case<config1>(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT(2600, solve_case<config1>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2286, solve_case<config2>(example));
  AOC_EXPECT_RESULT(86036, solve_case<config2>(input));

  AOC_RETURN_CHECK_RESULT();
}
