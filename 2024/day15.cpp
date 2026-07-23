// https://adventofcode.com/2024/day/15

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <span>

constexpr let wall = '#';
constexpr let empty = '.';
constexpr let box = 'O';
constexpr let robot = '@';

constexpr let up = '^';
constexpr let down = 'v';
constexpr let left = '<';
constexpr let right = '>';

using map_t = aoc::char_grid<>;
using directions_storage = Vec<aoc::facing_t>;

struct Input {
  map_t map;
  point robot_pos;
  directions_storage directions;
};

fn parse_directions(str directions) -> directions_storage {
  return directions |
         stdv::transform(&aoc::to_facing) |
         aoc::ranges::to<directions_storage>();
}

auto parse(String const& filename) -> Input {
  auto map = map_t{};
  auto robot_pos = point{};
  auto directions = directions_storage{};

  for (String line : aoc::views::read_lines(filename)) {
    if (line.empty()) {
      continue;
    }
    if (stdr::contains(std::array{up, down, left, right}, line[0])) {
      stdr::copy(parse_directions(line), std::back_inserter(directions));
    } else {
      let robot_x = line.find(robot);
      if (robot_x != String::npos) {
        robot_pos = point(robot_x, map.num_rows());
      }
      map.add_row(std::move(line));
    }
  }

  return {std::move(map), robot_pos, std::move(directions)};
}

fn move_robot(map_t map, point robot_pos,
              std::span<const aoc::facing_t> directions) -> map_t {
  AOC_ASSERT(robot == map.at(robot_pos.y, robot_pos.x),
             "Robot not located where expected");
  for (let direction : directions) {
    auto& robot_ref = map.at(robot_pos.y, robot_pos.x);
    let new_pos = robot_pos + aoc::get_diff(direction);
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
              AOC_UNREACHABLE("Invalid cell");
          }
        }
      } break;
      default:
        AOC_UNREACHABLE("Invalid cell");
    }
  }
  return map;
}

fn sum_coordinates(map_t const& map) -> int {
  return aoc::ranges::accumulate(
      aoc::views::indices_of(map) | stdv::filter([&](usize i) {
        return map.at_index(i) == box;
      }) | stdv::transform([&](usize i) {
        let pos = map.position(i);
        return 100 * pos.y + pos.x;
      }),
      0);
}

static_assert(104 == sum_coordinates(map_t{"#######"
                                           "#...O.."
                                           "#......",
                                           3, 7}));

template <bool>
fn solve_case(Input const& input) -> int {
  return sum_coordinates(
      move_robot(input.map, input.robot_pos, input.directions));
}

int main() {
  std::println("Part 1");
  let example = parse("day15.example");
  AOC_EXPECT_RESULT(10092, solve_case<false>(example));
  let example2 = parse("day15.example2");
  AOC_EXPECT_RESULT(2028, solve_case<false>(example2));
  let input = parse("day15.input");
  AOC_EXPECT_RESULT(1514353, solve_case<false>(input));

  std::println("Part 2");
  aoc::return_incomplete();
  // AOC_EXPECT_RESULT(281, solve_case<true>(example));
  // AOC_EXPECT_RESULT(53515, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
