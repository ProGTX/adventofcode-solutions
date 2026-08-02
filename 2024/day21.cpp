// https://adventofcode.com/2024/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <print>
#include <ranges>

using Input = Vec<String>;

fn parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) | aoc::collect_vec<String>();
}

constexpr str NUMERIC_KEYPAD = "789\n"
                               "456\n"
                               "123\n"
                               "X0A";
constexpr str DIRECTIONAL_KEYPAD = "X^A\n"
                                   "<v>";
constexpr let KEYPAD_GAP = 'X';
constexpr let ACTION = 'A';

constexpr usize NUM_DIRECTIONAL_ROBOTS = 2;

// Two robots on directional keypads, third on numeric one
struct SearchState {
  std::array<usize, NUM_DIRECTIONAL_ROBOTS + 1> robot_pos;
  char human_key;

  constexpr bool operator==(const SearchState&) const = default;
  constexpr auto operator<=>(const SearchState&) const = default;
};

fn keypad_grid(str keypad_text) {
  auto grid = aoc::char_grid<>{};
  for (let line : aoc::split(keypad_text, '\n')) {
    grid.add_row(line);
  }
  return grid;
};

fn solve_case1(Input const& input) -> u64 {
  let numeric_keypad = keypad_grid(NUMERIC_KEYPAD);
  let directional_keypad = keypad_grid(DIRECTIONAL_KEYPAD);

  // Moves an arm one step, or `None` if that leaves the keypad or hits the gap
  let move_arm = [](aoc::char_grid<> const& keypad, usize index,
                    char direction) -> Option<usize> {
    let pos = keypad.position(index);
    let diff = aoc::get_diff(aoc::to_facing(direction));
    let row = pos.y + diff.y;
    let col = pos.x + diff.x;
    if (!keypad.in_bounds(row, col)) {
      return None;
    }
    let new_index =
        keypad.linear_index(static_cast<usize>(row), static_cast<usize>(col));
    return (keypad.at_index(new_index) != KEYPAD_GAP) ? Option<usize>{new_index}
                                                      : None;
  };

  let human_press = [&](SearchState const& state,
                        char key) -> Option<SearchState> {
    auto new_state = state;
    new_state.human_key = key;
    if (key != ACTION) {
      // Anything but ACTION moves the first robot's arm directly
      let new_pos = move_arm(directional_keypad, state.robot_pos[0], key);
      if (!new_pos) {
        return None;
      }
      new_state.robot_pos[0] = *new_pos;
      return new_state;
    }
    // ACTION activates the key the first robot points at,
    // cascading inwards past every robot that is itself resting on ACTION
    let depth = static_cast<usize>(
        stdr::distance(state.robot_pos |
                       stdv::take(NUM_DIRECTIONAL_ROBOTS) |
                       stdv::take_while([&](usize index) {
                         return directional_keypad.at_index(index) == ACTION;
                       })));
    if (depth == NUM_DIRECTIONAL_ROBOTS) {
      // Every directional robot forwarded the press,
      // so the last robot types the numeric key it points at
      return new_state;
    }
    // The first robot not resting on ACTION holds a direction,
    // which moves the arm of the robot one level further in
    let direction = directional_keypad.at_index(state.robot_pos[depth]);
    let& inner_keypad = (depth + 1 < NUM_DIRECTIONAL_ROBOTS)
                            ? directional_keypad
                            : numeric_keypad;
    let new_pos = move_arm(inner_keypad, state.robot_pos[depth + 1], direction);
    if (!new_pos) {
      return None;
    }
    new_state.robot_pos[depth + 1] = *new_pos;
    return new_state;
  };

  // This assert allows us to just drop(1) in search
  AOC_ASSERT(directional_keypad.at_index(0) == KEYPAD_GAP,
             "Expected keypad gap at index 0");

  let neighbors = [&](SearchState const& state) {
    return directional_keypad.data() |
           stdv::drop(1) |
           aoc::views::transform_filter(
               [&](char human_key)
                   -> Option<aoc::dijkstra_neighbor_t<SearchState>> {
                 let next_state = human_press(state, human_key);
                 if (!next_state) {
                   return None;
                 }
                 return aoc::dijkstra_neighbor_t{*next_state, 1};
               });
  };

  let index_of_key = [](aoc::char_grid<> const& keypad, char key) {
    return keypad.data().find(key);
  };
  let dir_action_index = index_of_key(directional_keypad, ACTION);

  // Typing a code is one search per key,
  // each one starting where the previous one left the arms
  let shortest_sequence = [&](str code) {
    auto state = SearchState{
        .robot_pos = {dir_action_index, dir_action_index,
                      index_of_key(numeric_keypad, ACTION)},
        .human_key = KEYPAD_GAP,
    };
    auto human_keys = String{};
    for (let key : code) {
      let end = SearchState{
          .robot_pos = {dir_action_index, dir_action_index,
                        index_of_key(numeric_keypad, key)},
          .human_key = ACTION,
      };
      auto predecessors = aoc::predecessor_map<SearchState>{};
      aoc::shortest_distances_dijkstra(
          state, [&](SearchState const& s) { return s == end; }, neighbors,
          &predecessors);
      // `get_path` is in end-to-start order, and leaves out the start state
      let path = aoc::get_path(predecessors, end);
      for (let& s : path | stdv::reverse) {
        human_keys.push_back(s.human_key);
      }
      // The start's `human_key` only marks "nothing pressed yet",
      // keeping it distinct from `end` when a key gets typed twice in a row
      state = SearchState{.robot_pos = end.robot_pos, .human_key = KEYPAD_GAP};
    }
    return human_keys;
  };

  // The code without its trailing ACTION, so `029A` counts as 29
  let numeric = [](str code) {
    let digits_end = code.find_last_of("0123456789") + 1;
    return aoc::to_number<usize>(code.substr(0, digits_end));
  };

  return aoc::ranges::accumulate(
      input | stdv::transform([&](str code) {
        return static_cast<u64>(shortest_sequence(code).size() * numeric(code));
      }),
      u64{0});
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(126384, solve_case1(example));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(231564, solve_case1(input));

  std::println("Part 2");
  aoc::return_incomplete();
}
