// https://adventofcode.com/2016/day/17

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <utility>
#endif

/// The passcode for the vault
using Input = String;

constexpr let START = point{0, 0};
constexpr let VAULT = point{3, 3};

/// Every room has four doors, one per direction
constexpr let NUM_DOORS = 4uz;

/// A room, plus the passcode followed by the moves taken to reach it
///
/// The path alone already determines the room,
/// so it is all that needs to be hashed
struct State {
  point pos;
  String path;

  constexpr bool operator==(State const&) const = default;
  constexpr auto operator<=>(State const&) const = default;
};
template <>
struct std::hash<State> {
  // aoc::hash_bytes is a full wyhash round,
  // so unordered_dense can skip the mixing step it would otherwise add
  using is_avalanching = void;

  std::size_t operator()(State const& state) const noexcept {
    return aoc::hash_bytes(state.path.data(), state.path.size());
  }
};
static_assert(aoc::hashable<State>);

using Neighbors = aoc::static_vector<State, NUM_DOORS>;

/// The rooms only exist to keep the walk inside the four by four grid
constexpr let ROOMS = aoc::array_grid<u8, 4, 4>{};

auto parse(String const& filename) -> Input {
  return Input{aoc::trim(aoc::read_file(filename))};
}

fn get_neighbors(State const& state) {
  // Reaching the vault ends the walk, there is nowhere left to go
  if (state.pos == VAULT) {
    return aoc::dijkstra_uniform_neighbors(Neighbors{});
  }
  // The same hash decides all four doors of this room
  let hash = aoc::md5(state.path);
  return aoc::dijkstra_uniform_neighbors(
      ROOMS.basic_neighbor_positions(state.pos) |
      aoc::views::transform_filter([&](point neighbor) -> Option<State> {
        // The first four hex digits of the hash are the doors
        // up, down, left and right, in that order
        let[door, direction] = [&]() -> std::pair<usize, char> {
          if (neighbor.y < state.pos.y) {
            return {0, 'U'};
          } else if (neighbor.y > state.pos.y) {
            return {1, 'D'};
          } else if (neighbor.x < state.pos.x) {
            return {2, 'L'};
          } else {
            return {3, 'R'};
          }
        }();
        // Every hash byte holds two hex digits
        let digit = (hash[door / 2] >> (4 * (1 - (door % 2)))) & 0xf;
        // Only b through f, the digits above a, open a door
        if (digit <= 0xa) {
          return None;
        }
        return State{neighbor, state.path + direction};
      }) |
      aoc::ranges::to<Neighbors>());
}

/// The shortest path of moves from the top-left room to the vault
fn solve_case1(Input const& passcode) -> String {
  let start = State{START, passcode};
  let distances = aoc::shortest_distances_dijkstra(
      start, //
      [](State const& state) { return state.pos == VAULT; },
      [](State const& state) { return get_neighbors(state); });
  // Only the moves are the answer, not the passcode they are appended to
  auto vault_states = distances | stdv::filter([](let& entry) {
                        return entry.first.pos == VAULT;
                      });
  let closest = stdr::min_element(vault_states, {},
                                  [](let& entry) { return entry.second; });
  return closest->first.path.substr(passcode.size());
}

/// The length of the longest path of moves that still reaches the vault
///
/// Every move appends to the path, so no state can ever repeat:
/// the graph is a DAG, which is what `critical_distances` needs
fn solve_case2(Input const& passcode) -> u32 {
  let start = State{START, passcode};
  // Every path to the vault has to be walked to its end,
  // so the search cannot stop at the first one it settles
  let distances = aoc::critical_distances(
      start, [](State const& state) { return get_neighbors(state); });
  return static_cast<u32>(stdr::max(
      distances |
      stdv::filter([](let& entry) { return entry.first.pos == VAULT; }) |
      stdv::transform([](let& entry) { return entry.second; })));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT("DDRRRD", solve_case1(Input{"ihgpwlah"}));
  AOC_EXPECT_RESULT("DDUDRLRRUDRD", solve_case1(Input{"kglvqrro"}));
  let example = parse("day17.example");
  AOC_EXPECT_RESULT("DRURDRUDDLLDLUURRDULRLDUUDDDRR", solve_case1(example));
  let input = parse("day17.input");
  AOC_EXPECT_RESULT("RLDUDRDDRR", solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(370, solve_case2(Input{"ihgpwlah"}));
  AOC_EXPECT_RESULT(492, solve_case2(Input{"kglvqrro"}));
  AOC_EXPECT_RESULT(830, solve_case2(example));
  AOC_EXPECT_RESULT(590, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
