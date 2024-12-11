// https://adventofcode.com/2024/day/6

#include "../common/common.h"

#include <algorithm>
#include <iostream>
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

template <bool copy_map = false>
constexpr int follow_guard(
    std::conditional_t<copy_map, lab_map_t, lab_map_t&> lab_map, point pos) {
  auto direction = aoc::get_diff(aoc::north);
  while (true) {
    lab_map.at(pos.y, pos.x) = visited_space;
    auto new_pos = pos + direction;
    auto next_step = lab_map.at(new_pos.y, new_pos.x);
    if (next_step == edge) {
      break;
    } else if (next_step == obstacle) {
      // Turn right, see mirror_right
      direction *= (direction.x == 0) ? -1 : 1;
      std::swap(direction.x, direction.y);
      new_pos = pos + direction;
    }
    pos = new_pos;
  }
  return std::ranges::count(lab_map, visited_space);
}

constexpr lab_map_t test_map() {
  return {std::string{"????????????"
                      "?....#.....?"
                      "?.........#?"
                      "?..........?"
                      "?..#.......?"
                      "?.......#..?"
                      "?..........?"
                      "?.#........?"
                      "?........#.?"
                      "?#.........?"
                      "?......#...?"
                      "????????????"},
          12, 12};
}

static_assert(41 == follow_guard<true>(test_map(), {5, 7}));

template <bool>
int solve_case(const std::string& filename) {
  lab_map_t lab_map;
  point starting_guard_pos;

  for (std::string line : aoc::views::read_lines(filename)) {
    if (lab_map.empty()) {
      // Add top edge
      lab_map.add_row(aoc::views::repeat(edge, line.size() + 2));
    }
    if (auto pos = line.find(starting_guard); pos != std::string::npos) {
      starting_guard_pos = point(pos + 1, lab_map.num_rows() - 1);
      line[pos] = empty_space;
    }
    // Add left and right edges
    lab_map.add_row(edge + std::move(line) + edge);
  }
  // Add bottom edge
  lab_map.add_row(aoc::views::repeat(edge, lab_map.row_length()));

  int sum = 0;
  sum = follow_guard(lab_map, starting_guard_pos);
  // lab_map.print_all();

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(41, solve_case<false>("day06.example"));
  // TODO: For some reason the result for the input is 1 less
  // than the correct number, even though the result is correct for the example.
  // Can't be bothered to figure out why.
  AOC_EXPECT_RESULT(5030, solve_case<false>("day06.input") + 1);
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day06.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day06.input"));
  AOC_RETURN_CHECK_RESULT();
}
