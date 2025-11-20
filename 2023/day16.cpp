// https://adventofcode.com/2023/day/16

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

constexpr inline char empty = '.';
constexpr inline char mirror_right = '/';
constexpr inline char mirror_left = '\\';
constexpr inline char splitter_vertical = '|';
constexpr inline char splitter_horizontal = '-';
constexpr inline char energized = '#';

using machine_t = aoc::char_grid<>;

struct beam_t {
  point pos;
  point direction;

  constexpr bool operator==(const beam_t&) const noexcept = default;
};

constexpr int energize(const machine_t& machine, beam_t start) {
  machine_t energized_machine{machine};

  std::vector<beam_t> beams_tracker;
  std::vector<beam_t> beams;

  const auto split_beam = [&](point pos, point direction) {
    // Treat splitter as-if mirror_left
    std::swap(direction.x, direction.y);
    // Create a new beam as-if mirror_right
    beam_t beam2{pos - direction, -direction};
    if (!std::ranges::contains(beams_tracker, beam2)) {
      beams_tracker.push_back(beam2);
      beams.push_back(std::move(beam2));
    }
    // Modify current beam as-if mirror_left
    beam_t beam1{pos + direction, direction};
    if (!std::ranges::contains(beams_tracker, beam1)) {
      beams_tracker.push_back(std::move(beam1));
      return direction;
    }
    return point{};
  };

  beams.push_back(std::move(start));
  beams_tracker.push_back(beams.back());

  while (!beams.empty()) {
    auto [pos, direction] = aoc::pop_stack(beams);
    while (machine.in_bounds(pos.y, pos.x)) {
      auto value = machine.at(pos.y, pos.x);
      switch (value) {
        case empty:
          // Do nothing
          break;
        case mirror_left:
          std::swap(direction.x, direction.y);
          break;
        case mirror_right:
          direction = -direction;
          std::swap(direction.x, direction.y);
          break;
        case splitter_vertical:
          if (direction.y == 0) {
            direction = split_beam(pos, direction);
          } else {
            // Do nothing
          }
          break;
        case splitter_horizontal:
          if (direction.x == 0) {
            direction = split_beam(pos, direction);
          } else {
            // Do nothing
          }
          break;
        default:
          AOC_UNREACHABLE("Invalid value in the machine");
          break;
      }
      if (direction == point{}) {
        // Current beam already encountered, stop bouncing
        break;
      }
      energized_machine.modify(energized, pos.y, pos.x);
      pos += direction;
    }
  }

  return std::ranges::count(energized_machine, energized);
}

template <bool optimize>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  machine_t machine = aoc::read_char_grid(filename);

  int sum = 0;
  if constexpr (!optimize) {
    sum = energize(machine, beam_t{point{}, aoc::get_diff(aoc::east)});
  } else {
    AOC_ASSERT(machine.num_rows() == machine.row_length(),
               "We're assuming an n*n square grid for simplicity");
    std::vector<beam_t> beams;
    const auto n = static_cast<int>(machine.num_rows());
    for (int i = 0; i < n; ++i) {
      beams.emplace_back(point{0, i}, aoc::get_diff(aoc::east));
      beams.emplace_back(point{n - 1, i}, aoc::get_diff(aoc::west));
      beams.emplace_back(point{i, 0}, aoc::get_diff(aoc::south));
      beams.emplace_back(point{i, n - 1}, aoc::get_diff(aoc::north));
    }
    sum =
        std::ranges::max(beams | std::views::transform([&](const beam_t& beam) {
                           return energize(machine, beam);
                         }));
  }
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(46, (solve_case<false>("day16.example")));
  AOC_EXPECT_RESULT(7798, (solve_case<false>("day16.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(51, (solve_case<true>("day16.example")));
  AOC_EXPECT_RESULT(8026, (solve_case<true>("day16.input")));
  AOC_RETURN_CHECK_RESULT();
}
