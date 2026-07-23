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
constexpr let box_left = '[';
constexpr let box_right = ']';
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

fn push_wide_boxes_vertical(map_t& map, point new_pos, point diff) -> bool {
  // A wide box's two cells can each be blocked by a different box above/below,
  // so the stack to push is a tree, not a single line.
  // First, breadth-first collect every box in that tree without mutating,
  // failing fast if any of them is blocked by a wall.

  // Every box is identified by the position of its left ('[') cell,
  // even when we only have a reference to its right (']') half
  // That keeps `boxes` deduplicated below
  // (two adjacent boxes could otherwise be recorded once
  // via their left cell and once via their right cell)
  let box_left_root = [&](point pos) {
    return (map.at(pos.y, pos.x) == box_left) ? pos : pos - point{1, 0};
  };

  // `new_pos` is whichever of the two cells the robot bumped into
  let root = box_left_root(new_pos);
  auto boxes = aoc::flat_set<point>{};
  boxes.insert(root);
  auto frontier = Vec<point>{root};

  // BFS over layers: frontier holds the boxes discovered in the previous layer
  // whose neighbors (in the push direction) haven't been examined yet
  // `next_frontier` accumulates newly discovered boxes for the following layer
  // The loop ends once a layer discovers nothing new
  while (!frontier.empty()) {
    auto next_frontier = Vec<point>{};
    for (let box_pos : frontier) {
      // Check the cell directly ahead (in the push direction)
      // of each half of this box
      // The left half and the right half can each be blocked by a different box
      // so both must be inspected separately
      for (let cell : std::array{box_pos, box_pos + point{1, 0}}) {
        let ahead = cell + diff;
        switch (map.at(ahead.y, ahead.x)) {
          case wall:
            // Can't move stack of boxes into wall
            return false;
          case box_left:
          case box_right: {
            // Found another box in the way
            let ahead_box_pos = box_left_root(ahead);
            if (boxes.insert(ahead_box_pos).second) {
              // If box not already queued
              // (it may have been reached from a sibling box's other half,
              // or from a previous layer),
              // add it so its own neighbors get checked next layer
              next_frontier.push_back(ahead_box_pos);
            }
          } break;
          case empty:
            // Nothing blocking this half of the box,
            // no further boxes to add to the tree along this path
            break;
          default:
            AOC_UNREACHABLE("Invalid cell");
        }
      }
    }
    frontier = std::move(next_frontier);
  }

  // Every box in the tree has room to move by `diff`
  // A box's new position can coincide with another box's old position
  // so clear every old cell across the entire tree before writing any new cell
  // Otherwise a box moving into a cell could be immediately stepped on
  // by a not-yet-processed box vacating it
  for (let box_pos : boxes) {
    map.at(box_pos.y, box_pos.x) = empty;
    map.at(box_pos.y, box_pos.x + 1) = empty;
  }
  for (let box_pos : boxes) {
    let new_box_pos = box_pos + diff;
    map.at(new_box_pos.y, new_box_pos.x) = box_left;
    map.at(new_box_pos.y, new_box_pos.x + 1) = box_right;
  }

  // Boxes can be moved and have been,
  // but the caller is still responsible for moving the robot itself
  return true;
}

template <bool wide>
fn move_robot(map_t map, point robot_pos,
              std::span<const aoc::facing_t> directions) -> map_t {
  AOC_ASSERT(robot == map.at(robot_pos.y, robot_pos.x),
             "Robot not located where expected");

  let check_unreachable_cell = [](let& new_pos_ref) {
    if (!wide || ((new_pos_ref != box_left) && (new_pos_ref != box_right))) {
      AOC_UNREACHABLE("Invalid cell");
    }
  };

  let push_box_stack_simple = [&](point diff) {
    // Try to move stack of boxes by finding an empty space behind them
    let new_pos = robot_pos + diff;
    auto& new_pos_ref = map.at(new_pos.y, new_pos.x);
    auto& robot_ref = map.at(robot_pos.y, robot_pos.x);
    auto behind_boxes_pos = new_pos;
    bool searching = true;
    while (searching) {
      behind_boxes_pos += diff;
      auto& behind_boxes_ref = map.at(behind_boxes_pos.y, behind_boxes_pos.x);
      switch (behind_boxes_ref) {
        case wall:
          // Can't move stack of boxes into wall
          searching = false;
          break;
        case box:
          // Continue searching stack of boxes
          break;
        case empty:
          if constexpr (!wide) {
            // Move entire stack, but this can be achieved with just two steps
            std::swap(new_pos_ref, behind_boxes_ref);
            std::swap(robot_ref, new_pos_ref);
          } else {
            // Wide boxes alternate '[' / ']', so we need to rotate
            // the whole stack (plus the robot) by one step instead.
            // This path is only reached via horizontal movement,
            // so everything lies in one row.
            AOC_ASSERT(diff.y == 0,
                       "Cannot use this function for vertical movement");
            let robot_it = map.begin() + map.linear_index(robot_pos);
            let empty_it = map.begin() + map.linear_index(behind_boxes_pos);
            if (diff.x > 0) {
              aoc::ranges::rotate_right(robot_it, empty_it + 1);
            } else {
              aoc::ranges::rotate_left(empty_it, robot_it + 1);
            }
          }
          robot_pos = new_pos;
          searching = false;
          break;
        default:
          check_unreachable_cell(new_pos_ref);
          // If cell is valid (part 2), continue search
          break;
      }
    }
  };

  for (let direction : directions) {
    auto& robot_ref = map.at(robot_pos.y, robot_pos.x);
    let diff = aoc::get_diff(direction);
    let new_pos = robot_pos + diff;
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
      case box:
        push_box_stack_simple(diff);
        break;
      default: {
        check_unreachable_cell(new_pos_ref);
        if constexpr (wide) {
          if (diff.y == 0) {
            push_box_stack_simple(diff);
          } else if (push_wide_boxes_vertical(map, new_pos, diff)) {
            std::swap(robot_ref, new_pos_ref);
            robot_pos = new_pos;
          }
        }
      } break;
    }
  }
  return map;
}

template <bool wide = false>
fn sum_coordinates(map_t const& map) -> int {
  return aoc::ranges::accumulate(
      aoc::views::indices_of(map) | stdv::filter([&](usize i) {
        return map.at_index(i) == (wide ? box_left : box);
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

fn widen_cell(char c) -> std::array<char, 2> {
  switch (c) {
    case wall:
      return {wall, wall};
    case box:
      return {box_left, box_right};
    case empty:
      return {empty, empty};
    case robot:
      return {robot, empty};
    default:
      AOC_UNREACHABLE("Invalid cell");
  }
}

fn widen_map(map_t const& map) -> map_t {
  auto widened = map_t{};
  for (let row : aoc::views::indices(map.num_rows())) {
    widened.add_row(map.get_row(row) |
                    stdv::transform(&widen_cell) |
                    stdv::join |
                    aoc::collect_string());
  }
  return widened;
}

static_assert(widen_map(map_t{"#######"
                              "#...O.."
                              "#......",
                              3, 7})
                  .data() == "##############"
                             "##......[]...."
                             "##............");

static_assert(108 == sum_coordinates<true>( //
                         widen_map(map_t{"#######"
                                         "#...O.."
                                         "#......",
                                         3, 7})));

template <bool wide>
fn solve_case(Input const& input) -> int {
  return sum_coordinates<wide>(
      move_robot<wide>(wide ? widen_map(input.map) : input.map,
                       input.robot_pos * point{1 + static_cast<int>(wide), 1},
                       input.directions));
}

int main() {
  std::println("Part 1");
  let example = parse("day15.example");
  AOC_EXPECT_RESULT(10092, solve_case<false>(example));
  let example2 = parse("day15.example2");
  AOC_EXPECT_RESULT(2028, solve_case<false>(example2));
  let example3 = parse("day15.example3");
  AOC_EXPECT_RESULT(908, solve_case<false>(example3));
  let input = parse("day15.input");
  AOC_EXPECT_RESULT(1514353, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(9021, solve_case<true>(example));
  AOC_EXPECT_RESULT(1751, solve_case<true>(example2));
  AOC_EXPECT_RESULT(618, solve_case<true>(example3));
  AOC_EXPECT_RESULT(1533076, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
