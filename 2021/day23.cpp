// https://adventofcode.com/2021/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <array>
#include <optional>
#include <print>
#include <string>
#include <vector>

using Hallway = std::array<char, 11>;
using Rooms = std::array<std::array<char, 4>, 4>;
using Input = Rooms;

constexpr let HALLWAY =
    Hallway{'.', '.', 'X', '.', 'X', '.', 'X', '.', 'X', '.', '.'};
constexpr let EMPTY = HALLWAY[0];
constexpr let FORBIDDEN = HALLWAY[2];

fn hallway_index(usize room_index) -> usize { return room_index * 2 + 2; }

fn parse(String const& filename) -> Input {
  let lines = aoc::views::read_lines(filename, aoc::keep_spaces{}) |
              aoc::ranges::to<Vec<String>>();
  Input rooms;
  for (let room_index : Range{0uz, 4uz}) {
    let col = hallway_index(room_index) + 1;
    rooms[room_index] = {lines[3][col], lines[2][col], EMPTY, EMPTY};
  }
  return rooms;
}

struct Configuration {
  Hallway hallway;
  Rooms rooms;
  fn operator<=>(const Configuration&) const = default;
};

fn get_cost(char c) -> u32 {
  constexpr std::array<u32, 4> costs = {1, 10, 100, 1000};
  return costs[static_cast<usize>(c - 'A')];
}

fn correct_room(char c, usize room_index) -> bool {
  return static_cast<usize>(c - 'A') == room_index;
}

fn hallway_path(Hallway const& hallway, usize hall_index, usize room_index)
    -> Option<std::pair<usize, usize>> {
  let hall_above_room = hallway_index(room_index);
  let from = std::min(hall_above_room, hall_index);
  let to = std::max(hall_above_room, hall_index);
  for (let i : Range{from, to + 1}) {
    if ((i != hall_index) && (hallway[i] >= 'A') && (hallway[i] <= 'D')) {
      return None;
    }
  }
  return std::pair{from, to};
}

fn heuristic(Configuration const& config, usize room_size) -> i32 {
  auto cost = i32{};

  for (let h : Range{0uz, config.hallway.size()}) {
    let c = config.hallway[h];
    if ((c < 'A') || (c > 'D')) {
      continue;
    }
    let target_room = static_cast<usize>(c - 'A');
    let steps = std::abs(static_cast<i32>(h) -
                         static_cast<i32>(hallway_index(target_room))) +
                1;
    cost += steps * static_cast<i32>(get_cost(c));
  }

  for (let r : Range{0uz, 4uz}) {
    for (let s : Range{0uz, room_size}) {
      let c = config.rooms[r][s];
      if (c == EMPTY) {
        continue;
      }
      let target_room = static_cast<usize>(c - 'A');

      if (target_room == r) {
        // Settled if all slots below are also in the correct room
        bool below_ok = true;
        for (usize b = 0; b < s; ++b) {
          if (config.rooms[r][b] == EMPTY ||
              static_cast<usize>(config.rooms[r][b] - 'A') != r) {
            below_ok = false;
            break;
          }
        }
        if (below_ok)
          continue;
        // Must exit and re-enter (wrong amphipod is stuck below)
        let steps = static_cast<i32>(room_size - s) + 1;
        cost += steps * static_cast<i32>(get_cost(c));
      } else {
        let steps_out = static_cast<i32>(room_size - s);
        let h_steps = std::abs(static_cast<i32>(hallway_index(r)) -
                               static_cast<i32>(hallway_index(target_room)));
        cost += (steps_out + h_steps + 1) * static_cast<i32>(get_cost(c));
      }
    }
  }

  return cost;
}

fn solve(Rooms const& rooms, usize room_size) -> u32 {
  let start = Configuration{HALLWAY, rooms};

  auto end_rooms = Rooms{};
  for (auto& room : end_rooms) {
    room.fill(EMPTY);
  }
  for (usize room_index = 0; room_index < 4; ++room_index) {
    for (usize slot = 0; slot < room_size; ++slot) {
      end_rooms[room_index][slot] = static_cast<char>('A' + room_index);
    }
  }
  let end = Configuration{HALLWAY, std::move(end_rooms)};

  let distances = aoc::shortest_distances_astar(
      start,
      [&](Configuration const& current) {
        auto neighbors = Vec<aoc::dijkstra_neighbor_t<Configuration>>{};

        // Try moving from room to the hallway
        for (usize room_index = 0; room_index < 4; ++room_index) {
          let& room = current.rooms[room_index];

          // Find topmost occupied slot within room_size
          i32 top_slot = -1;
          for (i32 i = static_cast<i32>(room_size) - 1; i >= 0; --i) {
            if (room[i] != EMPTY) {
              top_slot = i;
              break;
            }
          }
          if (top_slot < 0) {
            continue;
          }

          // Skip if all occupied slots are already correct
          bool settled = true;
          for (u32 i = 0; i <= top_slot; ++i) {
            if (!correct_room(room[i], room_index)) {
              settled = false;
              break;
            }
          }
          if (settled) {
            continue;
          }

          let amphipod = room[top_slot];
          let steps_up = static_cast<i32>(room_size) - top_slot;
          auto new_room = room;
          new_room[top_slot] = EMPTY;

          for (usize hall_index = 0; hall_index < current.hallway.size();
               ++hall_index) {
            if (current.hallway[hall_index] != EMPTY) {
              continue;
            }
            let path = hallway_path(current.hallway, hall_index, room_index);
            if (!path) {
              continue;
            }
            let distance =
                (steps_up + static_cast<u32>(path->second - path->first)) *
                get_cost(amphipod);

            auto new_hallway = current.hallway;
            new_hallway[hall_index] = amphipod;

            auto new_rooms = current.rooms;
            new_rooms[room_index] = new_room;

            neighbors.emplace_back(Configuration{new_hallway, new_rooms},
                                   distance);
          }
        }

        // Try moving from the hallway to a room
        for (usize hall_index = 0; hall_index < current.hallway.size();
             ++hall_index) {
          let hall = current.hallway[hall_index];
          if ((hall == EMPTY) || (hall == FORBIDDEN)) {
            continue;
          }
          let amphipod = hall;
          let room_index = static_cast<usize>(amphipod - 'A');
          let& room = current.rooms[room_index];

          // Skip if any wrong amphipod is in the room
          bool has_wrong = false;
          for (usize i = 0; i < room_size; ++i) {
            if ((room[i] != EMPTY) && !correct_room(room[i], room_index)) {
              has_wrong = true;
              break;
            }
          }
          if (has_wrong) {
            continue;
          }

          // Find the deepest empty slot
          i32 target = -1;
          for (usize i = 0; i < room_size; ++i) {
            if (room[i] == EMPTY) {
              target = static_cast<i32>(i);
              break;
            }
          }
          if (target < 0) {
            continue;
          }

          let path = hallway_path(current.hallway, hall_index, room_index);
          if (!path) {
            continue;
          }
          let steps_down = static_cast<i32>(room_size) - target;
          let distance =
              (steps_down + static_cast<u32>(path->second - path->first)) *
              get_cost(amphipod);

          auto new_hallway = current.hallway;
          new_hallway[hall_index] = EMPTY;

          auto new_rooms = current.rooms;
          new_rooms[room_index][target] = amphipod;

          neighbors.emplace_back(Configuration{new_hallway, new_rooms},
                                 distance);
        }

        return neighbors;
      },
      [&](Configuration const& config) { return heuristic(config, room_size); },
      end);

  return distances.find(end)->second;
}

fn solve_case1(Rooms const& rooms) -> u32 { return solve(rooms, 2); }

fn solve_case2(Rooms const& rooms) -> u32 {
  //   #D#C#B#A#  -> slot 2
  //   #D#B#A#C#  -> slot 1
  constexpr std::array<char, 4> inserted_upper = {'D', 'C', 'B', 'A'};
  constexpr std::array<char, 4> inserted_lower = {'D', 'B', 'A', 'C'};
  Rooms rooms4;
  for (usize i = 0; i < 4; ++i) {
    rooms4[i] = {rooms[i][0], inserted_lower[i], inserted_upper[i],
                 rooms[i][1]};
  }
  return solve(rooms4, 4);
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(12521, solve_case1(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(14350, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(44169, solve_case2(example));
  AOC_EXPECT_RESULT(49742, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
