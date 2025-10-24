// https://adventofcode.com/2024/day/15

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const char wall = '#';
constexpr const char empty = '.';
constexpr const char box = 'O';
constexpr const char robot = '@';

constexpr const char up = '^';
constexpr const char down = 'v';
constexpr const char left = '<';
constexpr const char right = '>';

using map_t = aoc::char_grid<>;
using directions_storage = std::vector<aoc::facing_t>;

constexpr map_t move_robot(map_t map, point robot_pos,
                           std::span<const aoc::facing_t> directions) {
  AOC_ASSERT(robot == map.at(robot_pos.y, robot_pos.x),
             "Robot not located where expected");
  for (auto direction : directions) {
    auto& robot_ref = map.at(robot_pos.y, robot_pos.x);
    const auto new_pos = robot_pos + aoc::get_diff(direction);
    auto& new_pos_ref = map.at(new_pos.y, new_pos.x);
    switch (new_pos_ref) {
      case wall:
        // Can't move into wall
        break;
      case empty:
        // Move into empty space
        std::swap(robot_ref, new_pos_ref);
        robot_pos = new_pos;
        break;
      case box: {
        // Try to move stack of boxes by finding an empty space behind them
        auto behind_boxes_pos = new_pos;
        bool searching = true;
        while (searching) {
          behind_boxes_pos += aoc::get_diff(direction);
          auto& behind_boxes_ref =
              map.at(behind_boxes_pos.y, behind_boxes_pos.x);
          switch (behind_boxes_ref) {
            case wall:
              // Can't move stack of boxes into wall
              searching = false;
              break;
            case box:
              // Continue searching stack of boxes
              break;
            case empty:
              // Move entire stack, but this can be achieved with just two steps
              std::swap(new_pos_ref, behind_boxes_ref);
              std::swap(robot_ref, new_pos_ref);
              robot_pos = new_pos;
              searching = false;
              break;
            default:
              AOC_ASSERT(false, "Invalid cell");
          }
        }
      } break;
      default:
        AOC_ASSERT(false, "Invalid cell");
    }
  }
  return map;
}

constexpr directions_storage parse_directions(std::string_view directions) {
  return directions | std::views::transform(&aoc::to_facing) |
         aoc::ranges::to<directions_storage>();
}

constexpr int sum_coordinates(map_t map) {
  int sum = 0;
  for (int row = 0; row < map.num_rows(); ++row) {
    for (int col = 0; col < map.num_columns(); ++col) {
      if (map.at(row, col) == box) {
        sum += 100 * row + col;
      }
    }
  }
  return sum;
}

static_assert(104 == sum_coordinates(map_t{"#######"
                                           "#...O.."
                                           "#......",
                                           3, 7}));

template <bool words>
int solve_case(const std::string& filename) {
  map_t map;
  point robot_pos;
  directions_storage directions;

  for (std::string line : aoc::views::read_lines(filename)) {
    if (line.empty()) {
      continue;
    }
    if (std::ranges::contains(std::array{up, down, left, right}, line[0])) {
      std::ranges::copy(parse_directions(line), std::back_inserter(directions));
    } else {
      auto robot_x = line.find(robot);
      if (robot_x != std::string::npos) {
        robot_pos = point(robot_x, map.num_rows());
      }
      map.add_row(std::move(line));
    }
  }

  int sum = 0;
  sum = sum_coordinates(move_robot(map, robot_pos, directions));

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(10092, solve_case<false>("day15.example"));
  AOC_EXPECT_RESULT(2028, solve_case<false>("day15.example2"));
  AOC_EXPECT_RESULT(1514353, solve_case<false>("day15.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day15.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day15.input"));
  AOC_RETURN_CHECK_RESULT();
}
