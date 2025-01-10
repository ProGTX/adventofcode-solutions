// https://adventofcode.com/2024/day/6

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>

constexpr char empty_space = '.';
constexpr char obstacle = '#';
constexpr char starting_guard = '^';
constexpr char visited_space = 'X';

// We'll add edges to the map to remove the need for bounds checking
constexpr char edge = '?';

using lab_map_t = aoc::char_grid<>;

template <bool interfere, bool copy_map = false>
constexpr std::conditional_t<interfere, bool, int> follow_guard(
    std::conditional_t<
        copy_map, lab_map_t,
        std::conditional_t<interfere, const lab_map_t&, lab_map_t&>>
        lab_map,
    const point start_pos) {
  const auto start_direction = aoc::get_diff(aoc::north);
  auto pos = start_pos;
  auto direction = start_direction;
  std::conditional_t<interfere, aoc::flat_set<std::pair<point, point>>, int>
      visited;
  while (true) {
    if constexpr (!interfere) {
      lab_map.at(pos.y, pos.x) = visited_space;
    } else {
      auto new_node = std::pair{pos, direction};
      const auto [it, inserted] = visited.insert(new_node);
      if (!inserted) {
        // Detected a loop
        return true;
      }
    }
    auto new_pos = pos;
    char next_step = empty_space;
    // Try to turn right, but might need to do it multiple times, at most 3
    for (int i = 0; i < 3; ++i) {
      new_pos = pos + direction;
      next_step = lab_map.at(new_pos.y, new_pos.x);
      if (next_step == edge) {
        break;
      } else if (next_step == obstacle) {
        direction *= (direction.x == 0) ? -1 : 1;
        std::swap(direction.x, direction.y);
        new_pos = pos + direction;
      } else {
        break;
      }
    }
    if (next_step == edge) {
      break;
    }
    pos = new_pos;
  }
  if constexpr (!interfere) {
    return std::ranges::count(lab_map, visited_space);
  } else {
    return false;
  }
}

constexpr int interfere_with_guard(lab_map_t& lab_map, const point start_pos) {
  // Go through each point on the map and check if placing an obstacle there
  // would catch the guard in a loop
  // Ignore edges and the guard itself
  int count = 0;
  for (int row = 1; row < lab_map.num_rows() - 1; ++row) {
    for (int col = 1; col < lab_map.num_columns() - 1; ++col) {
      point pos{col, row};
      if (pos == start_pos) [[unlikely]] {
        continue;
      }
      if (lab_map.at(pos.y, pos.x) != empty_space) [[unlikely]] {
        continue;
      }
      lab_map.at(pos.y, pos.x) = obstacle;
      if (follow_guard<true>(lab_map, start_pos)) {
        ++count;
      }
      lab_map.at(pos.y, pos.x) = empty_space;
      std::ranges::replace(lab_map, visited_space, empty_space);
    }
  }
  return count;
}

constexpr lab_map_t test_map() {
  return {std::string{"????????????"
                      "?....#.....?"
                      "?.........#?"
                      "?..........?"
                      "?..#.......?"
                      "?.......#..?"
                      "?..........?"
                      "?.#..^.....?"
                      "?........#.?"
                      "?#.........?"
                      "?......#...?"
                      "????????????"},
          12, 12};
}

static_assert(41 == follow_guard<false, true>(test_map(), {5, 7}));

template <bool interfere>
int solve_case(const std::string& filename) {
  auto [lab_map, config] = aoc::read_char_grid(
      filename,
      {.padding = edge, .start_char = starting_guard, .end_char = {}});
  point starting_guard_pos = *config.start_pos;

  int sum = 0;
  if constexpr (!interfere) {
    sum = follow_guard<false>(lab_map, starting_guard_pos);
  } else {
    sum = interfere_with_guard(lab_map, starting_guard_pos);
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

void test_interference(point extra_obstacle) {
  constexpr point start_pos{5, 7};
  auto lab_map = test_map();
  aoc::println("TESTING",
               aoc::views::repeat('~', 64) | aoc::ranges::to<std::string>());
  lab_map.at(extra_obstacle.y, extra_obstacle.x) = obstacle;
  aoc::println("Check", follow_guard<true>(lab_map, start_pos));
  lab_map.at(extra_obstacle.y, extra_obstacle.x) = 'O';
  lab_map.print_all();
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(41, solve_case<false>("day06.example"));
  AOC_EXPECT_RESULT(5030, solve_case<false>("day06.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(6, solve_case<true>("day06.example"));
  // NOTE: Time required with different containers (on Ryzen 5950X):
  //    std::vector                       = 96    s
  //    std::set                          =  6    s
  //    aoc::flat_set<std::vector>        =  1.95 s
  //    aoc::flat_set<aoc::static_vector> =  1.78 s
  // NOTE: Running this on the same machine, but a different drive,
  //    the flat_set numbers are not so good: 24s for the vector version,
  //    and 100s for the static_vector.
  AOC_EXPECT_RESULT(1928, solve_case<true>("day06.input"));
  AOC_RETURN_CHECK_RESULT();
}
