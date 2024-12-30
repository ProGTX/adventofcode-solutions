// https://adventofcode.com/2024/day/14

#include "../common/common.h"

#include <array>
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct robot_t {
  point position;
  point velocity;
};

template <point grid_size, int seconds>
constexpr point move_robot(const robot_t robot) {
  return (robot.position + seconds * (grid_size + robot.velocity)) % grid_size;
}

static_assert(point{4, 1} == move_robot<{11, 7}, 1>({{2, 4}, {2, -3}}));
static_assert(point{6, 5} == move_robot<{11, 7}, 2>({{2, 4}, {2, -3}}));

template <point grid_size, int seconds>
constexpr std::vector<robot_t> move_all_robots(std::vector<robot_t> robots) {
  for (auto& robot : robots) {
    robot.position = move_robot<grid_size, seconds>(robot);
  }
  return robots;
}

template <point grid_size>
constexpr int safety_factor(std::span<const robot_t> robots) {
  constexpr const auto half = grid_size / 2;
  // The fifth quadrant is the one we ignore
  std::array<int, 5> quadrants{0, 0, 0, 0, 0};
  for (const auto& robot : robots) {
    const auto quadrant_id = [&]() {
      if ((robot.position.x == half.x) || (robot.position.y == half.y)) {
        // Ignore this robot
        return 4;
      }
      auto quadrant = robot.position / (half + point{1, 1});
      return 2 * quadrant.y + quadrant.x;
    }();
    ++quadrants[quadrant_id];
  }
  return aoc::ranges::fold_left(std::span{quadrants}.subspan(0, 4), 1,
                                std::multiplies{});
}

template <point grid_size, int seconds>
int solve_case(const std::string& filename) {
  std::vector<robot_t> robots;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [pos, vel] = aoc::split<std::array<std::string_view, 2>>(line, ' ');
    auto position = aoc::split<point>(pos.substr(2), ',');
    auto velocity = aoc::split<point>(vel.substr(2), ',');
    robots.push_back({position, velocity});
  }

  int sum = 0;
  sum = safety_factor<grid_size>(move_all_robots<grid_size, seconds>(robots));

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(12, (solve_case<{11, 7}, 100>("day14.example")));
  AOC_EXPECT_RESULT(221655456, (solve_case<{101, 103}, 100>("day14.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, (solve_case<{11,7},true>("day14.example")));
  // AOC_EXPECT_RESULT(53515, (solve_case<{101, 103},true>("day14.input")));
  AOC_RETURN_CHECK_RESULT();
}
